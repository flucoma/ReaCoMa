#pragma once
#include "../../dependencies/flucoma-core/include/flucoma/clients/rt/TransientSliceClient.hpp"
#include "FlucomaAlgorithmBase.h"

class TransientSliceAlgorithm
    : public FlucomaAlgorithm<fluid::client::NRTThreadedTransientSliceClient> {
  public:
    enum Params {
        kOrder = 0,
        kBlockSize,
        kPadding,
        kSkew,
        kThreshFwd,
        kThreshBack,
        kWinSize,
        kClump,
        kMinSliceLength,
        kNumParams
    };

    TransientSliceAlgorithm(ReacomaExtension *apiProvider);
    ~TransientSliceAlgorithm() override;

    const char *GetName() const override;
    void RegisterParameters() override;
    int GetNumAlgorithmParams() const override;

  protected:
    bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                   int frameCount, int sampleRate) override;
    bool HandleResults(MediaItem *item, MediaItem_Take *take, int numChannels,
                       int sampleRate) override;
};
