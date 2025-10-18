#include "AmpSliceAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"
#include "flucoma/clients/common/ParameterTypes.hpp"
#include "flucoma/clients/rt/AmpSliceClient.hpp"

AmpSliceAlgorithm::AmpSliceAlgorithm(ReacomaExtension *apiProvider)
    : SlicingAlgorithm<NRTThreadedAmpSliceClient>(apiProvider) {}

AmpSliceAlgorithm::~AmpSliceAlgorithm() = default;

void AmpSliceAlgorithm::RegisterParameters() {
    mBaseParamIdx = mApiProvider->NParams();

    for (int i = 0; i < kNumParams; ++i) {
        mApiProvider->AddParam();
    }
    mApiProvider->GetParam(mBaseParamIdx + kFastRampUpTime)
        ->InitInt("Fast Ramp Up Length (samples)", 3, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kFastRampDownTime)
        ->InitInt("Fast Ramp Down Length (samples)", 383, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kSlowRampUpTime)
        ->InitDouble("Slow Ramp Down Length (samples)", 2205, 1, 88200, 1);
    mApiProvider->GetParam(mBaseParamIdx + kSlowRampDownTime)
        ->InitDouble("Fast Ramp Down Length (samples)", 2205, 1, 88200, 1);
    mApiProvider->GetParam(mBaseParamIdx + kOnThreshold)
        ->InitInt("On Threshold (dB)", 19, -144, 144);
    mApiProvider->GetParam(mBaseParamIdx + kOffThreshold)
        ->InitInt("Off Threshold", 8, -144, 144);
    mApiProvider->GetParam(mBaseParamIdx + kSilenceThreshold)
        ->InitInt("Floor value (dB)", -70, -144, 144);
    mApiProvider->GetParam(mBaseParamIdx + kDebounce)
        ->InitInt("Minimum Slice Length (samples)", 1323, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kHiPassFreq)
        ->InitInt("High-Pass Filter Cutoff", 2000, 0, 10000);
}

bool AmpSliceAlgorithm::DoProcess(InputBufferT::type &sourceBuffer,
                                  int numChannels, int frameCount,
                                  int sampleRate) {
    int estimatedSlices = std::max(1, static_cast<int>(frameCount / 1024.0));
    auto outBuffer =
        std::make_shared<MemoryBufferAdaptor>(1, estimatedSlices, sampleRate);
    auto slicesOutputBuffer = fluid::client::BufferT::type(outBuffer);

    auto fastRampUpTime =
        mApiProvider->GetParam(mBaseParamIdx + kFastRampUpTime)->Value();
    auto fastRampDownTime =
        mApiProvider->GetParam(mBaseParamIdx + kFastRampDownTime)->Value();
    auto slowRampUpTime =
        mApiProvider->GetParam(mBaseParamIdx + kSlowRampUpTime)->Value();
    auto slowRampDownTime =
        mApiProvider->GetParam(mBaseParamIdx + kSlowRampDownTime)->Value();
    auto onThreshold =
        mApiProvider->GetParam(mBaseParamIdx + kOnThreshold)->Value();
    auto offThreshold =
        mApiProvider->GetParam(mBaseParamIdx + kOffThreshold)->Value();
    auto floorValue =
        mApiProvider->GetParam(mBaseParamIdx + kSilenceThreshold)->Value();
    auto debounceTime =
        mApiProvider->GetParam(mBaseParamIdx + kDebounce)->Value();
    auto hiPassFreq =
        mApiProvider->GetParam(mBaseParamIdx + kHiPassFreq)->Value();

    mParams.template set<0>(std::move(sourceBuffer), nullptr);
    mParams.template set<1>(std::move(LongT::type(0)), nullptr);
    mParams.template set<2>(std::move(LongT::type(-1)), nullptr);
    mParams.template set<3>(std::move(LongT::type(0)), nullptr);
    mParams.template set<4>(std::move(LongT::type(-1)), nullptr);
    mParams.template set<5>(std::move(slicesOutputBuffer), nullptr);
    mParams.template set<6>(std::move(LongT::type(fastRampUpTime)), nullptr);
    mParams.template set<7>(std::move(LongT::type(fastRampDownTime)), nullptr);
    mParams.template set<8>(std::move(LongT::type(slowRampUpTime)), nullptr);
    mParams.template set<9>(std::move(LongT::type(slowRampDownTime)), nullptr);
    mParams.template set<10>(std::move(FloatT::type(onThreshold)), nullptr);
    mParams.template set<11>(std::move(FloatT::type(offThreshold)), nullptr);
    mParams.template set<12>(std::move(FloatT::type(floorValue)), nullptr);
    mParams.template set<13>(std::move(LongT::type(debounceTime)), nullptr);
    mParams.template set<14>(std::move(FloatT::type(hiPassFreq)), nullptr);

    mClient = NRTThreadedAmpSliceClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();
    return result.ok();
}

const char *AmpSliceAlgorithm::GetName() const { return "Amp Slice"; }

int AmpSliceAlgorithm::GetNumAlgorithmParams() const { return kNumParams; }

BufferT::type &AmpSliceAlgorithm::GetSlicesBuffer() {
    return mParams.template get<5>();
}