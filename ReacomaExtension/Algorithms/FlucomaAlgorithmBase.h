#pragma once

#include "../../dependencies/flucoma-core/include/flucoma/clients/common/FluidBaseClient.hpp"
#include "../../dependencies/flucoma-core/include/flucoma/clients/common/FluidContext.hpp"
#include "../../dependencies/flucoma-core/include/flucoma/clients/common/ParameterTypes.hpp"
#include "../../dependencies/flucoma-core/include/flucoma/clients/common/Result.hpp"
#include "../VectorBufferAdaptor.h"
#include "IAlgorithm.h"

#include "wdltypes.h"
#include "reaper_plugin_functions.h"

#include <filesystem>
#include <iomanip>
#include <sstream>

using namespace fluid;
using namespace client;

class ReacomaExtension;

template <typename ClientType> class FlucomaAlgorithm : public IAlgorithm {
public:
    FlucomaAlgorithm(ReacomaExtension *apiProvider)
        : IAlgorithm(apiProvider), mContext{},
          mParams{ClientType::getParameterDescriptors(),
                  FluidDefaultAllocator()},
          mClient{mParams, mContext} {}

    virtual ~FlucomaAlgorithm() override = default;

    bool StartProcessItemAsync(MediaItem *item) override final {

        SetMediaItemInfo_Value(item, "C_LOCK", true);
        UpdateTimeline();

        if (!item || !mApiProvider)
            return false;
        mIsFinishedFlag = false;
        mProgress = 0.0;

        MediaItem_Take *take = GetActiveTake(item);
        if (!take)
            return false;

        PCM_source *source = GetMediaItemTake_Source(take);
        if (!source)
            return false;

        const int sampleRate = GetMediaSourceSampleRate(source);
        const int numChannels = GetMediaSourceNumChannels(source);
        const double itemLength = GetMediaItemInfo_Value(item, "D_LENGTH");
        const double playrate = GetMediaItemTakeInfo_Value(take, "D_PLAYRATE");
        const double takeOffset =
            GetMediaItemTakeInfo_Value(take, "D_STARTOFFS");
        const double sourceDuration = source->GetLength();

        const double effectiveTakeDuration = itemLength * playrate;
        const double actualDurationToProcess =
            std::min(effectiveTakeDuration, sourceDuration - takeOffset);
        const int frameCount =
            static_cast<int>(sampleRate * actualDurationToProcess);

        if (frameCount <= 0 || numChannels <= 0)
            return false;

        std::vector<double> allChannelsAsDouble(frameCount * numChannels);
        PCM_source_transfer_t transfer{};
        transfer.time_s = takeOffset;
        transfer.samplerate = static_cast<double>(sampleRate);
        transfer.nch = numChannels;
        transfer.length = frameCount;
        transfer.samples = allChannelsAsDouble.data();
        source->GetSamples(&transfer);

        std::vector<float> allChannelsAsFloat(allChannelsAsDouble.begin(),
                                              allChannelsAsDouble.end());
        auto inputBuffer = InputBufferT::type(new fluid::VectorBufferAdaptor(
            allChannelsAsFloat, numChannels, frameCount, sampleRate));

        mItemForAsync = item;
        mTakeForAsync = take;
        mNumChannelsForAsync = numChannels;
        mSampleRateForAsync = sampleRate;

        if (!DoProcess(inputBuffer, numChannels, frameCount, sampleRate)) {
            return false;
        }

        return true;
    }

    bool IsFinished() override final {
        if (mIsFinishedFlag)
            return true;
        Result result;
        ProcessState processState = mClient.checkProgress(result);
        mProgress = mClient.progress();

        if (processState == fluid::client::ProcessState::kDone ||
            processState == fluid::client::ProcessState::kDoneStillProcessing) {
            mIsFinishedFlag = true;
            mProgress = 1.0;
        }
        return mIsFinishedFlag;
    }

    bool FinalizeProcess(MediaItem *item) override final {
        if (!mItemForAsync || item != mItemForAsync)
            return false;

        bool success = HandleResults(mItemForAsync, mTakeForAsync,
                                     mNumChannelsForAsync, mSampleRateForAsync);

        mItemForAsync = nullptr;
        mTakeForAsync = nullptr;

        SetMediaItemInfo_Value(item, "C_LOCK", false);

        if (success) {
            UpdateTimeline();
        }

        return success;
    }

    void Cancel() override final { mClient.cancel(); }

    double GetProgress() override final { return mProgress; }

    bool SupportsSegmentation() override { return true; }

    bool SupportsRegions() override { return true; }

    bool CreatesTakes() override { return false; }

protected:
    void CreateRegionsFromSlices(MediaItem *item, BufferT::type &slices,
                                 int sampleRate);
    virtual bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                           int frameCount, int sampleRate) = 0;
    virtual bool HandleResults(MediaItem *item, MediaItem_Take *take,
                               int numChannels, int sampleRate) = 0;

protected:
    FluidContext mContext;
    typename ClientType::ParamSetType mParams;
    ClientType mClient;

private:
    MediaItem *mItemForAsync = nullptr;
    MediaItem_Take *mTakeForAsync = nullptr;
    int mNumChannelsForAsync = 0;
    int mSampleRateForAsync = 0;
    bool mIsFinishedFlag = false;
    double mProgress = 0.0;
};

template <typename ClientType>
class AudioOutputAlgorithm : public FlucomaAlgorithm<ClientType> {
protected:
    using FlucomaAlgorithm<ClientType>::mApiProvider;

    AudioOutputAlgorithm(ReacomaExtension *apiProvider)
        : FlucomaAlgorithm<ClientType>(apiProvider) {}

    void AddOutputToTake(MediaItem *item, BufferT::type output, int sampleRate,
                         const std::string &suffix) {
        if (!output)
            return;

        fluid::client::BufferAdaptor::ReadAccess bufferReader(output.get());
        if (!bufferReader.exists() || !bufferReader.valid())
            return;

        auto numFrames = bufferReader.numFrames();
        auto numChans = bufferReader.numChans();

        std::vector<std::vector<ReaSample>> channelData(
            numChans, std::vector<ReaSample>(numFrames));
        std::vector<ReaSample *> pointerArray(numChans);
        const float *sourceData = bufferReader.allFrames().data();

        for (int i = 0; i < numChans; ++i) {
            for (int j = 0; j < numFrames; ++j) {
                channelData[i][j] =
                    static_cast<ReaSample>(sourceData[j * numChans + i]);
            }
            pointerArray[i] = channelData[i].data();
        }

        char originalFilePathCStr[4096] = "";
        auto activeTake = GetActiveTake(item);
        if (activeTake) {
            auto takeSource = GetMediaItemTake_Source(activeTake);
            if (takeSource) {
                auto srcParent = GetMediaSourceParent(takeSource);
                GetMediaSourceFileName(srcParent ? srcParent : takeSource,
                                       originalFilePathCStr,
                                       sizeof(originalFilePathCStr));
            }
        }

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");

        std::filesystem::path originalPath(originalFilePathCStr);
        auto parentDir = originalPath.parent_path();
        auto stem = originalPath.stem().string();

        std::filesystem::path reacomaFolder = parentDir / "reacoma";
        std::filesystem::create_directory(reacomaFolder);

        std::string takeName = stem + "_" + ss.str() + "_" + suffix;
        std::string newFilename = takeName + ".wav";
        std::filesystem::path outputFilePath = reacomaFolder / newFilename;

        struct WavConfig {
            char fourcc[4];
            int bit_depth;
        };
        WavConfig config;
        memcpy(config.fourcc, "evaw", 4);
        config.bit_depth = 32;

        PCM_sink *sink = PCM_Sink_CreateEx(
            nullptr, outputFilePath.string().c_str(), (const char *)&config,
            sizeof(config), numChans, sampleRate, true);
        if (sink) {
            sink->WriteDoubles(pointerArray.data(), numFrames, numChans, 0, 1);
            delete sink;
        }

        PCM_source *newSource =
            PCM_Source_CreateFromFile(outputFilePath.u8string().c_str());
        if (newSource) {
            MediaItem_Take *newTake = AddTakeToMediaItem(item);
            if (newTake) {
                GetSetMediaItemTakeInfo(newTake, "P_SOURCE", newSource);
                GetSetMediaItemTakeInfo(newTake, "P_NAME",
                                        (char *)takeName.c_str());
            }
        }
    }

public:
    bool SupportsSegmentation() { return false; }
    bool SupportsRegions() { return false; }
    bool CreatesTakes() { return true; }
};