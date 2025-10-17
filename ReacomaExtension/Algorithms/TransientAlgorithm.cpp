#include "TransientAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"

TransientAlgorithm::TransientAlgorithm(ReacomaExtension *apiProvider)
    : AudioOutputAlgorithm<NRTThreadedTransientsClient>(apiProvider) {}

TransientAlgorithm::~TransientAlgorithm() = default;

void TransientAlgorithm::RegisterParameters() {
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
}

bool TransientAlgorithm::DoProcess(InputBufferT::type &sourceBuffer,
                                   int numChannels, int frameCount,
                                   int sampleRate) {
    auto order = mApiProvider->GetParam(mBaseParamIdx + kOrder)->Value();
    auto blockSize =
        mApiProvider->GetParam(mBaseParamIdx + kBlockSize)->Value();
    auto padding = mApiProvider->GetParam(mBaseParamIdx + kPadding)->Value();
    auto skew = mApiProvider->GetParam(mBaseParamIdx + kSkew)->Value();
    auto fwd = mApiProvider->GetParam(mBaseParamIdx + kThreshFwd)->Value();
    auto bwd = mApiProvider->GetParam(mBaseParamIdx + kThreshBack)->Value();
    auto winSize = mApiProvider->GetParam(mBaseParamIdx + kWinSize)->Value();
    auto clumpLength = mApiProvider->GetParam(mBaseParamIdx + kClump)->Value();

    auto transMemoryBuffer = std::make_shared<MemoryBufferAdaptor>(
        numChannels, frameCount, sampleRate);
    auto resMemoryBuffer = std::make_shared<MemoryBufferAdaptor>(
        numChannels, frameCount, sampleRate);
    auto transOutputBuffer = fluid::client::BufferT::type(transMemoryBuffer);
    auto resOutputBuffer = fluid::client::BufferT::type(resMemoryBuffer);

    mParams.template set<0>(std::move(sourceBuffer), nullptr);
    mParams.template set<1>(std::move(LongT::type(0)), nullptr);
    mParams.template set<2>(std::move(LongT::type(-1)), nullptr);
    mParams.template set<3>(std::move(LongT::type(0)), nullptr);
    mParams.template set<4>(std::move(LongT::type(-1)), nullptr);
    mParams.template set<5>(std::move(transOutputBuffer),
                            nullptr);                             // transients
    mParams.template set<6>(std::move(resOutputBuffer), nullptr); // residual
    mParams.template set<7>(std::move(LongT::type(order)), nullptr);
    mParams.template set<8>(std::move(LongT::type(blockSize)), nullptr);
    mParams.template set<9>(std::move(LongT::type(padding)), nullptr);
    mParams.template set<10>(std::move(FloatT::type(skew)), nullptr);
    mParams.template set<11>(std::move(FloatT::type(fwd)), nullptr);
    mParams.template set<12>(std::move(FloatT::type(bwd)), nullptr);
    mParams.template set<13>(std::move(LongT::type(winSize)), nullptr);
    mParams.template set<14>(std::move(LongT::type(clumpLength)), nullptr);

    mClient = NRTThreadedTransientsClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();

    return result.ok();
}

bool TransientAlgorithm::HandleResults(MediaItem *item, MediaItem_Take *take,
                                       int numChannels, int sampleRate) {
    auto transOutputBuffer = mParams.template get<5>();
    auto resOutputBuffer = mParams.template get<6>();

    AddOutputToTake(item, transOutputBuffer, sampleRate, "transients");
    AddOutputToTake(item, resOutputBuffer, sampleRate, "residual");
    return true;
}

const char *TransientAlgorithm::GetName() const {
    return "Transient Separation";
}

int TransientAlgorithm::GetNumAlgorithmParams() const { return kNumParams; }
