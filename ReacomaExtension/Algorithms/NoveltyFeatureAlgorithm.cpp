#include "NoveltyFeatureAlgorithm.h"
#include "IPlugParameter.h"
#include "ReacomaExtension.h"

NoveltyFeatureAlgorithm::NoveltyFeatureAlgorithm(ReacomaExtension *apiProvider)
    : FlucomaAlgorithm<NRTThreadedNoveltyFeatureClient>(apiProvider) {}

NoveltyFeatureAlgorithm::~NoveltyFeatureAlgorithm() = default;

void NoveltyFeatureAlgorithm::RegisterParameters() {
    mBaseParamIdx = mApiProvider->NParams();

    for (int i = 0; i < NoveltyFeatureAlgorithm::kNumParams; ++i) {
        mApiProvider->AddParam();
    }

    mApiProvider->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kKernelSize)
        ->InitInt("Kernel Size", 3, 3, 101);

    mApiProvider->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kFilterSize)
        ->InitInt("Filter Size", 1, 1, 31);

    mApiProvider->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kWindowSize)
        ->InitInt("Window Size", 1024, 2, 65536);

    mApiProvider->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kHopSize)
        ->InitInt("Hop Size", 512, 2, 65536);

    mApiProvider->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kFFTSize)
        ->InitInt("FFT Size", 1024, 2, 65536);

    IParam *algoParam = mApiProvider->GetParam(
        mBaseParamIdx + NoveltyFeatureAlgorithm::kAlgorithm);
    algoParam->InitEnum("Algorithm", NoveltyFeatureAlgorithm::kSpectrum,
                        NoveltyFeatureAlgorithm::kNumAlgorithmOptions - 1);
    algoParam->SetDisplayText(NoveltyFeatureAlgorithm::kSpectrum, "Spectrum");
    algoParam->SetDisplayText(NoveltyFeatureAlgorithm::kMFCC, "MFCC");
    algoParam->SetDisplayText(NoveltyFeatureAlgorithm::kChroma, "Chroma");
    algoParam->SetDisplayText(NoveltyFeatureAlgorithm::kPitch, "Pitch");
    algoParam->SetDisplayText(NoveltyFeatureAlgorithm::kLoudness, "Loudness");
}

bool NoveltyFeatureAlgorithm::DoProcess(InputBufferT::type &sourceBuffer,
                                        int numChannels, int frameCount,
                                        int sampleRate) {
    auto kernelsize =
        mApiProvider
            ->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kKernelSize)
            ->Value();
    auto filtersize =
        mApiProvider
            ->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kFilterSize)
            ->Value();
    auto windowSize =
        mApiProvider
            ->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kWindowSize)
            ->Value();
    auto hopSize =
        mApiProvider
            ->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kHopSize)
            ->Value();
    auto fftSize =
        mApiProvider
            ->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kFFTSize)
            ->Value();
    auto algorithm =
        mApiProvider
            ->GetParam(mBaseParamIdx + NoveltyFeatureAlgorithm::kAlgorithm)
            ->Value();

    int estimatedSamples =
        std::max(1, static_cast<int>(frameCount / hopSize)) + 2;
    auto outBuffer =
        std::make_shared<MemoryBufferAdaptor>(1, estimatedSamples, sampleRate);
    auto slicesOutputBuffer = fluid::client::BufferT::type(outBuffer);

    if (static_cast<int>(kernelsize) % 2 == 0)
        kernelsize += 1;
    if (static_cast<int>(filtersize) % 2 == 0)
        filtersize += 1;

    mParams.template set<0>(std::move(sourceBuffer), nullptr);
    mParams.template set<1>(LongT::type(0), nullptr);
    mParams.template set<2>(LongT::type(-1), nullptr);
    mParams.template set<3>(LongT::type(0), nullptr);
    mParams.template set<4>(LongT::type(-1), nullptr);
    mParams.template set<5>(std::move(slicesOutputBuffer), nullptr);
    mParams.template set<6>(LongT::type(algorithm), nullptr);
    mParams.template set<7>(LongRuntimeMaxParam(kernelsize, kernelsize),
                            nullptr);
    mParams.template set<8>(LongRuntimeMaxParam(filtersize, filtersize),
                            nullptr);
    mParams.template set<10>(
        fluid::client::FFTParams(windowSize, hopSize, fftSize), nullptr);

    mClient = NRTThreadedNoveltyFeatureClient(mParams, mContext);
    mClient.setSynchronous(false);
    mClient.enqueue(mParams);
    Result result = mClient.process();

    return result.ok();
}

bool NoveltyFeatureAlgorithm::HandleResults(MediaItem *item,
                                            MediaItem_Take *take,
                                            int numChannels, int sampleRate) {
    auto processedSlicesBuffer = mParams.template get<5>();
    BufferAdaptor::ReadAccess reader(processedSlicesBuffer.get());

    if (!reader.exists() || !reader.valid()) {
        return false;
    }

    // int markerCount = GetNumTakeMarkers(take);
    // for (int i = markerCount - 1; i >= 0; i--) {
    //     DeleteTakeMarker(take, i);
    // }

    // double itemLength = GetMediaItemInfo_Value(item, "D_LENGTH");
    // auto view = reader.samps(0);
    // for (fluid::index i = 0; i < view.size(); i++) {
    //     if (view(i) > 0) {
    //         double markerTimeInSeconds =
    //             static_cast<double>(view(i)) / sampleRate;
    //         if (markerTimeInSeconds < itemLength) {
    //             SetTakeMarker(take, -1, "", &markerTimeInSeconds, nullptr);
    //         }
    //     }
    // }
    return true;
}

const char *NoveltyFeatureAlgorithm::GetName() const {
    return "Novelty Feature";
}

int NoveltyFeatureAlgorithm::GetNumAlgorithmParams() const {
    return kNumParams;
}
