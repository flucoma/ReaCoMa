#pragma once

#include "FlucomaAlgorithmBase.h"
#include "ReacomaExtension.h"

template <typename ClientType>
class SlicingAlgorithm : public FlucomaAlgorithm<ClientType> {
public:
    SlicingAlgorithm(ReacomaExtension *apiProvider)
        : FlucomaAlgorithm<ClientType>(apiProvider) {}

protected:
    virtual BufferT::type &GetSlicesBuffer() = 0;

private:
    bool HandleResults(MediaItem *item, MediaItem_Take *take, int numChannels,
                       int sampleRate) override {
        BufferT::type &slices = GetSlicesBuffer();
        if (!slices)
            return false;

        // The mode-switching logic now lives here, in one place.
        auto mode = this->mApiProvider->GetCurrentMode();
        if (mode == ReacomaExtension::Mode::Regions) {
            CreateRegionsFromSlices(item, slices, sampleRate);
        } else {
            CreateTakeMarkers(item, take, slices, sampleRate);
        }
        return true;
    }

    // The helper methods are now part of this base class.
    void CreateTakeMarkers(MediaItem *item, MediaItem_Take *take,
                           BufferT::type &slices, int sampleRate) {
        BufferAdaptor::ReadAccess reader(slices.get());
        if (!reader.exists() || !reader.valid())
            return;

        int markerCount = GetNumTakeMarkers(take);
        for (int i = markerCount - 1; i >= 0; i--)
            DeleteTakeMarker(take, i);

        double itemLength = GetMediaItemInfo_Value(item, "D_LENGTH");
        auto view = reader.samps(0);
        for (fluid::index i = 0; i < view.size(); i++) {
            if (view(i) > 0) {
                double markerTimeInSeconds =
                    static_cast<double>(view(i)) / sampleRate;
                if (markerTimeInSeconds < itemLength)
                    SetTakeMarker(take, -1, "", &markerTimeInSeconds, nullptr);
            }
        }
    }

    void CreateRegionsFromSlices(MediaItem *item, BufferT::type &slices,
                                 int sampleRate) {
        BufferAdaptor::ReadAccess reader(slices.get());
        if (!reader.exists() || !reader.valid())
            return;

        ReaProject *project = GetItemProjectContext(item);
        if (!project)
            return;

        double itemPos = GetMediaItemInfo_Value(item, "D_POSITION");
        double itemLen = GetMediaItemInfo_Value(item, "D_LENGTH");

        auto view = reader.samps(0);
        std::vector<double> sliceTimes;

        // Add start and end of the item as boundaries
        sliceTimes.push_back(0.0);
        sliceTimes.push_back(itemLen);

        // Collect all slice points and convert from samples to seconds
        for (fluid::index i = 0; i < view.size(); i++) {
            if (view(i) > 0) {
                double timeInSeconds =
                    static_cast<double>(view(i)) / sampleRate;
                if (timeInSeconds < itemLen)
                    sliceTimes.push_back(timeInSeconds);
            }
        }

        // Sort and remove duplicates to ensure clean regions
        std::sort(sliceTimes.begin(), sliceTimes.end());
        sliceTimes.erase(std::unique(sliceTimes.begin(), sliceTimes.end()),
                         sliceTimes.end());

        // Loop through pairs of slice points to create regions
        for (size_t i = 0; i < sliceTimes.size() - 1; ++i) {
            double regionStart = itemPos + sliceTimes[i];
            double regionEnd = itemPos + sliceTimes[i + 1];

            // Avoid creating zero-length regions
            if (regionEnd > regionStart) {
                char name[64];
                snprintf(name, sizeof(name), "Region %zu", i + 1);
                AddProjectMarker2(project, true, regionStart, regionEnd, name,
                                  -1, 0);
            }
        }
    }
};