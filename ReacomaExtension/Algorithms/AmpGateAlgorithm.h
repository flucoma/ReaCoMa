#pragma once
#include "../../dependencies/flucoma-core/include/flucoma/clients/rt/AmpGateClient.hpp"
#include "FlucomaAlgorithmBase.h"

class AmpGateAlgorithm
    : public FlucomaAlgorithm<fluid::client::NRTThreadedAmpGateClient> {
  public:
    enum Params {
        kRampUpTime = 0,
        kRampDownTime,
        kOnThreshold,
        kOffThreshold,
        kMinEventDuration,
        kMinSilenceDuration,
        kMinTimeAboveThreshold,
        kMinTimeBelowThreshold,
        kUpwardLookupTime,
        kDownwardLookupTime,
        kHiPassFreq,
        kNumParams
    };

    AmpGateAlgorithm(ReacomaExtension *apiProvider);
    ~AmpGateAlgorithm() override;

    const char *GetName() const override;
    void RegisterParameters() override;
    int GetNumAlgorithmParams() const override;

  protected:
    bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                   int frameCount, int sampleRate) override;
    bool HandleResults(MediaItem *item, MediaItem_Take *take, int numChannels,
                       int sampleRate) override;
};
