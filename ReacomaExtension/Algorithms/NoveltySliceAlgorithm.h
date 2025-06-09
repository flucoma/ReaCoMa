#pragma once
#include "../../dependencies/flucoma-core/include/flucoma/clients/rt/NoveltySliceClient.hpp"
#include "SlicingAlgorithm.h"

class NoveltySliceAlgorithm
    : public SlicingAlgorithm<fluid::client::NRTThreadingNoveltySliceClient> {
public:
    enum Params {
        kThreshold = 0,
        kKernelSize,
        kFilterSize,
        kMinSliceLength,
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

    NoveltySliceAlgorithm(ReacomaExtension *apiProvider);
    ~NoveltySliceAlgorithm() override;

    const char *GetName() const override;
    void RegisterParameters() override;
    int GetNumAlgorithmParams() const override;

protected:
    BufferT::type &GetSlicesBuffer() override;
    bool DoProcess(InputBufferT::type &sourceBuffer, int numChannels,
                   int frameCount, int sampleRate) override;
};
