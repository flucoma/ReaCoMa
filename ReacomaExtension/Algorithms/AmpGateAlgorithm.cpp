#include "AmpGateAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"

AmpGateAlgorithm::AmpGateAlgorithm(ReacomaExtension *apiProvider)
    : FlucomaAlgorithm<NRTThreadedAmpGateClient>(apiProvider) {}

AmpGateAlgorithm::~AmpGateAlgorithm() = default;

void AmpGateAlgorithm::RegisterParameters() {
    mBaseParamIdx = mApiProvider->NParams();
    for (int i = 0; i < kNumParams; ++i) {
        mApiProvider->AddParam();
    }
    mApiProvider->GetParam(mBaseParamIdx + kRampUpTime)
        ->InitInt("Ramp Up Length (samples)", 10, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kRampDownTime)
        ->InitInt("Ramp Down Length (samples)", 10, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kOnThreshold)
        ->InitDouble("On Threshold (dB)", -12, -144, 144, 0.1);
    mApiProvider->GetParam(mBaseParamIdx + kOffThreshold)
        ->InitDouble("Off Threshold (dB)", -24, -144, 144, 0.1);
    mApiProvider->GetParam(mBaseParamIdx + kMinEventDuration)
        ->InitInt("Minimum Slice Length", 1, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kMinSilenceDuration)
        ->InitInt("Minimum Silence Length", 1, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kMinTimeAboveThreshold)
        ->InitInt("Minimum Length Above", 1, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kMinTimeBelowThreshold)
        ->InitInt("Minimum Length Below", 1, 1, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kUpwardLookupTime)
        ->InitInt("Lookback", 0, 0, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kDownwardLookupTime)
        ->InitInt("Lookahead", 0, 0, 88200);
    mApiProvider->GetParam(mBaseParamIdx + kHiPassFreq)
        ->InitInt("High-Pass Filter Cutoff (Hz)", 85, 0, 5000);
}

bool AmpGateAlgorithm::DoProcess(InputBufferT::type &sourceBuffer,
                                 int numChannels, int frameCount,
                                 int sampleRate) {
    int estimatedSlices = std::max(1, static_cast<int>(frameCount / 1024.0));
    auto outBuffer =
        std::make_shared<MemoryBufferAdaptor>(1, estimatedSlices, sampleRate);
    auto slicesOutputBuffer = fluid::client::BufferT::type(outBuffer);

    auto rampUpTime =
        mApiProvider->GetParam(mBaseParamIdx + kRampUpTime)->Value();
    auto rampDownTime =
        mApiProvider->GetParam(mBaseParamIdx + kRampDownTime)->Value();
    auto onThreshold =
        mApiProvider->GetParam(mBaseParamIdx + kOnThreshold)->Value();
    auto offThreshold =
        mApiProvider->GetParam(mBaseParamIdx + kOffThreshold)->Value();
    auto minEventDuration =
        mApiProvider->GetParam(mBaseParamIdx + kMinEventDuration)->Value();
    auto minSilenceDuration =
        mApiProvider->GetParam(mBaseParamIdx + kMinSilenceDuration)->Value();
    auto minTimeAboveThreshold =
        mApiProvider->GetParam(mBaseParamIdx + kMinTimeAboveThreshold)->Value();
    auto minTimeBelowThreshold =
        mApiProvider->GetParam(mBaseParamIdx + kMinTimeBelowThreshold)->Value();
    auto upwardLookupTime =
        mApiProvider->GetParam(mBaseParamIdx + kUpwardLookupTime)->Value();
    auto downwardLookupTime =
        mApiProvider->GetParam(mBaseParamIdx + kDownwardLookupTime)->Value();
    auto hiPassFreq =
        mApiProvider->GetParam(mBaseParamIdx + kHiPassFreq)->Value();

    mParams.template set<0>(std::move(sourceBuffer), nullptr);
    mParams.template set<1>(LongT::type(0), nullptr);
    mParams.template set<2>(LongT::type(-1), nullptr);
    mParams.template set<3>(LongT::type(0), nullptr);
    mParams.template set<4>(LongT::type(-1), nullptr);
    mParams.template set<5>(std::move(slicesOutputBuffer), nullptr);
    mParams.template set<6>(LongT::type(rampUpTime), nullptr);
    mParams.template set<7>(LongT::type(rampDownTime), nullptr);
    mParams.template set<8>(FloatT::type(onThreshold), nullptr);
    mParams.template set<9>(FloatT::type(offThreshold), nullptr);
    mParams.template set<10>(LongT::type(minEventDuration), nullptr);
    mParams.template set<11>(LongT::type(minSilenceDuration), nullptr);
    mParams.template set<12>(LongT::type(minTimeAboveThreshold), nullptr);
    mParams.template set<13>(LongT::type(minTimeBelowThreshold), nullptr);
    mParams.template set<14>(LongT::type(upwardLookupTime), nullptr);
    mParams.template set<15>(LongT::type(downwardLookupTime), nullptr);
    mParams.template set<16>(LongT::type(hiPassFreq), nullptr);

    mClient = NRTThreadedAmpGateClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();
    return result.ok();
}

bool AmpGateAlgorithm::HandleResults(MediaItem *item, MediaItem_Take *take,
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
    auto onsetView = reader.samps(0);
    for (fluid::index i = 0; i < onsetView.size(); i++) {
        if (onsetView(i) > 0) {
            double markerTimeInSeconds =
                static_cast<double>(onsetView(i)) / sampleRate;
            if (markerTimeInSeconds < itemLength) {
                SetTakeMarker(take, -1, "", &markerTimeInSeconds, nullptr);
            }
        }
    }

    // Offset marker colour
    int r = 255;
    int g = 0;
    int b = 0;

    int color = ColorToNative(r, g, b) | 0x1000000;

    auto offsetView = reader.samps(1);
    for (fluid::index i = 0; i < offsetView.size(); i++) {
        if (offsetView(i) > 0) {
            double markerTimeInSeconds =
                static_cast<double>(offsetView(i)) / sampleRate;
            if (markerTimeInSeconds < itemLength) {
                SetTakeMarker(take, -1, "", &markerTimeInSeconds, &color);
            }
        }
    }
    return true;
}

const char *AmpGateAlgorithm::GetName() const { return "Onset Slice"; }

int AmpGateAlgorithm::GetNumAlgorithmParams() const { return kNumParams; }
