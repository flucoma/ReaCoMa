#pragma once
#include "../../dependencies/flucoma-core/include/flucoma/clients/rt/TransientClient.hpp"
#include "FlucomaAlgorithmBase.h"

class TransientAlgorithm
    : public AudioOutputAlgorithm<fluid::client::NRTThreadedTransientsClient> {
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
        kNumParams
    };

    TransientAlgorithm(ReacomaExtension *apiProvider);
    ~TransientAlgorithm() override;

    const char *GetName() const override;
    void RegisterParameters() override;
    int GetNumAlgorithmParams() const override;

  protected:
    bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                   int frameCount, int sampleRate) override;
    bool HandleResults(MediaItem *item, MediaItem_Take *take, int numChannels,
                       int sampleRate) override;
};
