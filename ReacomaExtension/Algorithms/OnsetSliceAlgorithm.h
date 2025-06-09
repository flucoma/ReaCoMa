#pragma once
#include "../../dependencies/flucoma-core/include/flucoma/clients/rt/OnsetSliceClient.hpp"
#include "FlucomaAlgorithmBase.h"

class OnsetSliceAlgorithm
    : public FlucomaAlgorithm<fluid::client::NRTThreadingOnsetSliceClient> {
  public:
    enum Params {
        kMetric = 0,
        kThreshold,
        kMinSliceLength,
        kFilterSize,
        kFrameDelta,
        kWindowSize,
        kHopSize,
        kFFTSize,
        kNumParams
    };

    OnsetSliceAlgorithm(ReacomaExtension *apiProvider);
    ~OnsetSliceAlgorithm() override;

    const char *GetName() const override;
    void RegisterParameters() override;
    int GetNumAlgorithmParams() const override;

  protected:
    bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                   int frameCount, int sampleRate) override;
    bool HandleResults(MediaItem *item, MediaItem_Take *take, int numChannels,
                       int sampleRate) override;
};