#include "NMFAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"

NMFAlgorithm::NMFAlgorithm(ReacomaExtension *apiProvider)
    : AudioOutputAlgorithm<NRTThreadedNMFClient>(apiProvider) {}

NMFAlgorithm::~NMFAlgorithm() = default;

void NMFAlgorithm::RegisterParameters() {
    mBaseParamIdx = mApiProvider->NParams();
    for (int i = 0; i < kNumParams; ++i) {
        mApiProvider->AddParam();
    }
    mApiProvider->GetParam(mBaseParamIdx + kComponents)
        ->InitInt("Number of Components", 2, 2, 10);
    mApiProvider->GetParam(mBaseParamIdx + kIterations)
        ->InitInt("Number of Iterations", 100, 1, 1000);

    mApiProvider->GetParam(mBaseParamIdx + NMFAlgorithm::kWindowSize)
        ->InitInt("Window Size", 1024, 2, 65536);

    mApiProvider->GetParam(mBaseParamIdx + NMFAlgorithm::kHopSize)
        ->InitInt("Hop Size", 512, 2, 65536);

    mApiProvider->GetParam(mBaseParamIdx + NMFAlgorithm::kFFTSize)
        ->InitInt("FFT Size", 1024, 2, 65536);
}

bool NMFAlgorithm::DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                             int frameCount, int sampleRate) {
    auto componentsParam =
        mApiProvider->GetParam(mBaseParamIdx + kComponents)->Value();
    auto iterationsParam =
        mApiProvider->GetParam(mBaseParamIdx + kIterations)->Value();

    auto windowSize =
        mApiProvider->GetParam(mBaseParamIdx + NMFAlgorithm::kWindowSize)
            ->Value();
    auto hopSize =
        mApiProvider->GetParam(mBaseParamIdx + NMFAlgorithm::kHopSize)->Value();
    auto fftSize =
        mApiProvider->GetParam(mBaseParamIdx + NMFAlgorithm::kFFTSize)->Value();

    auto resynthMemoryBuffer = std::make_shared<MemoryBufferAdaptor>(
        numChannels * componentsParam, frameCount, sampleRate);
    auto resynthOutputBuffer =
        fluid::client::BufferT::type(resynthMemoryBuffer);

    mParams.template set<0>(std::move(sourceBuffer), nullptr);
    mParams.template set<1>(std::move(LongT::type(0)), nullptr);
    mParams.template set<2>(std::move(LongT::type(-1)), nullptr);
    mParams.template set<3>(std::move(LongT::type(0)), nullptr);
    mParams.template set<4>(std::move(LongT::type(-1)), nullptr);
    mParams.template set<5>(std::move(resynthOutputBuffer), nullptr);
    mParams.template set<6>(std::move(LongT::type(1)), nullptr);
    mParams.template set<7>(nullptr, nullptr);
    mParams.template set<8>(std::move(LongT::type(0)), nullptr);
    mParams.template set<9>(nullptr, nullptr);
    mParams.template set<10>(std::move(LongT::type(0)), nullptr);
    mParams.template set<11>(std::move(componentsParam), nullptr);
    mParams.template set<12>(std::move(iterationsParam), nullptr);
    mParams.template set<13>(
        std::move(fluid::client::FFTParams(windowSize, hopSize, fftSize,
                                           std::max(windowSize, fftSize))),
        nullptr);

    mClient = NRTThreadedNMFClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();

    return result.ok();
}

bool NMFAlgorithm::HandleResults(MediaItem *item, MediaItem_Take *take,
                                 int numChannels, int sampleRate) {
    auto resynthOutputBuffer =
        mParams.template get<5>(); // Get the buffer we created in DoProcess
    AddOutputToTake(item, resynthOutputBuffer, sampleRate, "nmf");
    return true;
}

const char *NMFAlgorithm::GetName() const {
    return "Non-negative Matrix Factorisation";
}

int NMFAlgorithm::GetNumAlgorithmParams() const { return kNumParams; }
