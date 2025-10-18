#pragma once
#include "flucoma/clients/rt/AmpSliceClient.hpp"
#include "SlicingAlgorithm.h"

class AmpSliceAlgorithm
    : public SlicingAlgorithm<fluid::client::NRTThreadedAmpSliceClient> {
public:
    enum Params {
        kFastRampUpTime = 0,
        kFastRampDownTime,
        kSlowRampUpTime,
        kSlowRampDownTime,
        kOnThreshold,
        kOffThreshold,
        kSilenceThreshold,
        kDebounce,
        kHiPassFreq,
        kNumParams
    };

    AmpSliceAlgorithm(ReacomaExtension *apiProvider);
    ~AmpSliceAlgorithm() override;

    const char *GetName() const override;
    void RegisterParameters() override;
    int GetNumAlgorithmParams() const override;

protected:
    BufferT::type &GetSlicesBuffer() override;
    bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                   int frameCount, int sampleRate) override;
};
