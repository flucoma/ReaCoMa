#include "OnsetSliceAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"

OnsetSliceAlgorithm::OnsetSliceAlgorithm(ReacomaExtension *apiProvider)
    : FlucomaAlgorithm<NRTThreadingOnsetSliceClient>(apiProvider) {}

OnsetSliceAlgorithm::~OnsetSliceAlgorithm() = default;

void OnsetSliceAlgorithm::RegisterParameters() {
    mBaseParamIdx = mApiProvider->NParams();
    for (int i = 0; i < kNumParams; ++i) {
        mApiProvider->AddParam();
    }
    mApiProvider->GetParam(mBaseParamIdx + kMetric)->InitInt("Metric", 0, 0, 9);
    mApiProvider->GetParam(mBaseParamIdx + kThreshold)
        ->InitDouble("Threshold", 0.5, 0.0, 1.0, 0.01);
    mApiProvider->GetParam(mBaseParamIdx + kFrameDelta)
        ->InitInt("Frame Delta", 0, 0, 10);
    mApiProvider->GetParam(mBaseParamIdx + kFilterSize)
        ->InitInt("Filter Size", 5, 1, 101);
    mApiProvider->GetParam(mBaseParamIdx + kMinSliceLength)
        ->InitInt("Min Length", 2, 1, 1000);
    mApiProvider->GetParam(mBaseParamIdx + kWindowSize)
        ->InitInt("Window Size", 1024, 2, 65536);
    mApiProvider->GetParam(mBaseParamIdx + kHopSize)
        ->InitInt("Hop Size", 512, 2, 65536);
    mApiProvider->GetParam(mBaseParamIdx + kFFTSize)
        ->InitInt("FFT Size", 1024, 2, 65536);
}

bool OnsetSliceAlgorithm::DoProcess(InputBufferT::type &sourceBuffer,
                                    int numChannels, int frameCount,
                                    int sampleRate) {
    int estimatedSlices = std::max(1, static_cast<int>(frameCount / 1024.0));
    auto outBuffer =
        std::make_shared<MemoryBufferAdaptor>(1, estimatedSlices, sampleRate);
    auto slicesOutputBuffer = fluid::client::BufferT::type(outBuffer);

    auto metric = mApiProvider->GetParam(mBaseParamIdx + kMetric)->Value();
    auto threshold =
        mApiProvider->GetParam(mBaseParamIdx + kThreshold)->Value();
    auto filterSize =
        mApiProvider->GetParam(mBaseParamIdx + kFilterSize)->Value();
    auto frameDelta =
        mApiProvider->GetParam(mBaseParamIdx + kFrameDelta)->Value();
    auto minLength =
        mApiProvider->GetParam(mBaseParamIdx + kMinSliceLength)->Value();
    auto windowSize =
        mApiProvider->GetParam(mBaseParamIdx + kWindowSize)->Value();
    auto hopSize = mApiProvider->GetParam(mBaseParamIdx + kHopSize)->Value();
    auto fftSize = mApiProvider->GetParam(mBaseParamIdx + kFFTSize)->Value();

    if (static_cast<int>(filterSize) % 2 == 0)
        filterSize += 1;

    mParams.template set<0>(std::move(sourceBuffer), nullptr);
    mParams.template set<1>(LongT::type(0), nullptr);
    mParams.template set<2>(LongT::type(-1), nullptr);
    mParams.template set<3>(LongT::type(0), nullptr);
    mParams.template set<4>(LongT::type(-1), nullptr);
    mParams.template set<5>(std::move(slicesOutputBuffer), nullptr);
    mParams.template set<6>(LongT::type(metric), nullptr);
    mParams.template set<7>(FloatT::type(threshold), nullptr);
    mParams.template set<8>(LongT::type(minLength), nullptr);
    mParams.template set<9>(LongRuntimeMaxParam(filterSize, filterSize),
                            nullptr);
    mParams.template set<10>(LongT::type(frameDelta), nullptr);
    mParams.template set<11>(
        fluid::client::FFTParams(windowSize, hopSize, fftSize,
                                 std::max(windowSize, fftSize)),
        nullptr);

    mClient = NRTThreadingOnsetSliceClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();
    return result.ok();
}

bool OnsetSliceAlgorithm::HandleResults(MediaItem *item, MediaItem_Take *take,
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

const char *OnsetSliceAlgorithm::GetName() const { return "Onset Slice"; }

int OnsetSliceAlgorithm::GetNumAlgorithmParams() const { return kNumParams; }
