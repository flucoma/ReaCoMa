#include "HPSSAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"

HPSSAlgorithm::HPSSAlgorithm(ReacomaExtension *apiProvider)
    : AudioOutputAlgorithm<NRTThreadedHPSSClient>(apiProvider) {}

HPSSAlgorithm::~HPSSAlgorithm() = default;

void HPSSAlgorithm::RegisterParameters() {
    mBaseParamIdx = mApiProvider->NParams();

    for (int i = 0; i < HPSSAlgorithm::kNumParams; ++i) {
        mApiProvider->AddParam();
    }

    mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kHarmFilterSize)
        ->InitInt("Harmonic Filter Size", 17, 3, 101);

    mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kPercFilterSize)
        ->InitInt("Percussive Filter Size", 31, 3, 101);

    mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kWindowSize)
        ->InitInt("Window Size", 1024, 2, 65536);

    mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kHopSize)
        ->InitInt("Hop Size", 512, 2, 65536);

    mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kFFTSize)
        ->InitInt("FFT Size", 1024, 2, 65536);
}

bool HPSSAlgorithm::DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                              int frameCount, int sampleRate) {
    auto harmFilterSizeParam =
        mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kHarmFilterSize)
            ->Value();
    auto percFilterSizeParam =
        mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kPercFilterSize)
            ->Value();

    auto windowSize =
        mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kWindowSize)
            ->Value();
    auto hopSize =
        mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kHopSize)
            ->Value();
    auto fftSize =
        mApiProvider->GetParam(mBaseParamIdx + HPSSAlgorithm::kFFTSize)
            ->Value();

    auto harmMemoryBuffer =
        std::make_shared<MemoryBufferAdaptor>(1, frameCount, sampleRate);
    auto percMemoryBuffer =
        std::make_shared<MemoryBufferAdaptor>(1, frameCount, sampleRate);
    auto harmOutputBuffer = fluid::client::BufferT::type(harmMemoryBuffer);
    auto percOutputBuffer = fluid::client::BufferT::type(percMemoryBuffer);

    if (static_cast<int>(harmFilterSizeParam) % 2 == 0)
        harmFilterSizeParam += 1;
    if (static_cast<int>(percFilterSizeParam) % 2 == 0)
        percFilterSizeParam += 1;

    mParams.template set<0>(std::move(sourceBuffer), nullptr);     // source
    mParams.template set<1>(LongT::type(0), nullptr);              // startChan
    mParams.template set<2>(LongT::type(-1), nullptr);             // numChans
    mParams.template set<3>(LongT::type(0), nullptr);              // startFrame
    mParams.template set<4>(LongT::type(-1), nullptr);             // numFrames
    mParams.template set<5>(std::move(harmOutputBuffer), nullptr); // harmonic
    mParams.template set<6>(std::move(percOutputBuffer), nullptr); // percussive
    mParams.template set<7>(nullptr, nullptr);
    mParams.template set<8>(
        LongRuntimeMaxParam(harmFilterSizeParam, harmFilterSizeParam), nullptr);
    mParams.template set<9>(
        LongRuntimeMaxParam(percFilterSizeParam, percFilterSizeParam), nullptr);
    mParams.template set<10>(LongT::type(0), nullptr);
    mParams.template set<11>(FloatPairsArrayT::type(0, 1, 1, 1), nullptr);
    mParams.template set<12>(FloatPairsArrayT::type(1, 0, 1, 1), nullptr);
    mParams.template set<13>(
        fluid::client::FFTParams(windowSize, hopSize, fftSize,
                                 std::max(windowSize, fftSize)),
        nullptr);

    mClient = NRTThreadedHPSSClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();

    if (!result.ok()) {
        ShowConsoleMsg("NMF processing failed.\n");
        return false;
    }
    return true;
}

bool HPSSAlgorithm::HandleResults(MediaItem *item, MediaItem_Take *take,
                                  int numChannels, int sampleRate) {
    auto harmOutputBuffer = mParams.template get<5>();
    auto percOutputBuffer = mParams.template get<6>();

    AddOutputToTake(item, harmOutputBuffer, sampleRate, "harmonic");
    AddOutputToTake(item, percOutputBuffer, sampleRate, "percussive");
    return true;
}

const char *HPSSAlgorithm::GetName() const {
    return "Harmonic Percussive Source Separation";
}

int HPSSAlgorithm::GetNumAlgorithmParams() const { return kNumParams; }
