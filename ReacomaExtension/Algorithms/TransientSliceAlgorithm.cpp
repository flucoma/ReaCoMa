#include "TransientSliceAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"

TransientSliceAlgorithm::TransientSliceAlgorithm(ReacomaExtension *apiProvider)
    : FlucomaAlgorithm<NRTThreadedTransientSliceClient>(apiProvider) {}

TransientSliceAlgorithm::~TransientSliceAlgorithm() = default;

void TransientSliceAlgorithm::RegisterParameters() {
    mBaseParamIdx = mApiProvider->NParams();
    for (int i = 0; i < kNumParams; ++i) {
        mApiProvider->AddParam();
    }
    mApiProvider->GetParam(mBaseParamIdx + kOrder)
        ->InitInt("Order", 100, 1, 1000);
    mApiProvider->GetParam(mBaseParamIdx + kBlockSize)
        ->InitInt("Block Size", 256, 2, 4096);
    mApiProvider->GetParam(mBaseParamIdx + kPadding)
        ->InitInt("Padding", 128, 0, 2048);
    mApiProvider->GetParam(mBaseParamIdx + kSkew)
        ->InitDouble("Skew", 0, -10, 10, 0.1);
    mApiProvider->GetParam(mBaseParamIdx + kThreshFwd)
        ->InitDouble("Forward Threshold", 2, 0, 10, 0.01); // no max
    mApiProvider->GetParam(mBaseParamIdx + kThreshBack)
        ->InitDouble("Backward Threshold", 1.1, 0, 10, 0.01);
    mApiProvider->GetParam(mBaseParamIdx + kWinSize)
        ->InitInt("Window Size", 14, 0, 100);
    mApiProvider->GetParam(mBaseParamIdx + kClump)
        ->InitInt("Clump Length", 25, 0, 100);
    mApiProvider->GetParam(mBaseParamIdx + kMinSliceLength)
        ->InitInt("Minimum Slice Length", 1000, 0, 10000);
}

bool TransientSliceAlgorithm::DoProcess(InputBufferT::type &sourceBuffer,
                                        int numChannels, int frameCount,
                                        int sampleRate) {
    int estimatedSlices = std::max(1, static_cast<int>(frameCount / 1024.0));
    auto outBuffer =
        std::make_shared<MemoryBufferAdaptor>(1, estimatedSlices, sampleRate);
    auto slicesOutputBuffer = fluid::client::BufferT::type(outBuffer);

    auto order = mApiProvider->GetParam(mBaseParamIdx + kOrder)->Value();
    auto blockSize =
        mApiProvider->GetParam(mBaseParamIdx + kBlockSize)->Value();
    auto padding = mApiProvider->GetParam(mBaseParamIdx + kPadding)->Value();
    auto skew = mApiProvider->GetParam(mBaseParamIdx + kSkew)->Value();
    auto fwd = mApiProvider->GetParam(mBaseParamIdx + kThreshFwd)->Value();
    auto bwd = mApiProvider->GetParam(mBaseParamIdx + kThreshBack)->Value();
    auto winSize = mApiProvider->GetParam(mBaseParamIdx + kWinSize)->Value();
    auto clumpLength = mApiProvider->GetParam(mBaseParamIdx + kClump)->Value();
    auto minSliceLength =
        mApiProvider->GetParam(mBaseParamIdx + kMinSliceLength)->Value();

    mParams.template set<0>(std::move(sourceBuffer), nullptr);
    mParams.template set<1>(LongT::type(0), nullptr);
    mParams.template set<2>(LongT::type(-1), nullptr);
    mParams.template set<3>(LongT::type(0), nullptr);
    mParams.template set<4>(LongT::type(-1), nullptr);
    mParams.template set<5>(std::move(slicesOutputBuffer), nullptr);
    mParams.template set<6>(LongT::type(order), nullptr);
    mParams.template set<7>(LongT::type(blockSize), nullptr);
    mParams.template set<8>(LongT::type(padding), nullptr);
    mParams.template set<9>(FloatT::type(skew), nullptr);
    mParams.template set<10>(FloatT::type(fwd), nullptr);
    mParams.template set<11>(FloatT::type(bwd), nullptr);
    mParams.template set<12>(LongT::type(winSize), nullptr);
    mParams.template set<13>(LongT::type(clumpLength), nullptr);
    mParams.template set<14>(LongT::type(minSliceLength), nullptr);

    mClient = NRTThreadedTransientSliceClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();
    return result.ok();
}

bool TransientSliceAlgorithm::HandleResults(MediaItem *item,
                                            MediaItem_Take *take,
                                            int numChannels, int sampleRate) {
    auto processedSlicesBuffer = mParams.template get<5>();
    BufferAdaptor::ReadAccess reader(processedSlicesBuffer.get());

    if (!reader.exists() || !reader.valid())
        return false;

    int markerCount = GetNumTakeMarkers(take);
    for (int i = markerCount - 1; i >= 0; i--) {
        DeleteTakeMarker(take, i);
    }

    double itemLength = GetMediaItemInfo_Value(item, "D_LENGTH");
    auto view = reader.samps(0);
    for (fluid::index i = 0; i < view.size(); i++) {
        if (view(i) > 0) {
            double markerTimeInSeconds =
                static_cast<double>(view(i)) / sampleRate;
            if (markerTimeInSeconds < itemLength) {
                SetTakeMarker(take, -1, "", &markerTimeInSeconds, nullptr);
            }
        }
    }
    return true;
}

const char *TransientSliceAlgorithm::GetName() const {
    return "Transient Slice";
}

int TransientSliceAlgorithm::GetNumAlgorithmParams() const {
    return kNumParams;
}
