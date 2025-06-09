#pragma once
#include "FlucomaAlgorithmBase.h"
#include "../../dependencies/flucoma-core/include/flucoma/clients/rt/NoveltyFeatureClient.hpp"

class NoveltyFeatureAlgorithm
    : public FlucomaAlgorithm<fluid::client::NRTThreadedNoveltyFeatureClient> {
  public:
    enum Params {
        kKernelSize = 0,
        kFilterSize,
        kWindowSize,
        kHopSize,
        kFFTSize,
        kAlgorithm,
        kNumParams
    };

    enum EAlgorithmOptions {
        kSpectrum = 0,
        kMFCC,
        kChroma,
        kPitch,
        kLoudness,
        kNumAlgorithmOptions
    };

    NoveltyFeatureAlgorithm(ReacomaExtension *apiProvider);
    ~NoveltyFeatureAlgorithm() override;

    const char *GetName() const override;
    void RegisterParameters() override;
    int GetNumAlgorithmParams() const override;

  protected:
    bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                   int frameCount, int sampleRate) override;
    bool HandleResults(MediaItem *item, MediaItem_Take *take, int numChannels,
                       int sampleRate) override;
};
