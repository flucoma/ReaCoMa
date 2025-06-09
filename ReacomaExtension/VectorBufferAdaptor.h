#pragma once
#include "../dependencies/flucoma-core/include/flucoma/clients/common/BufferAdaptor.hpp"
#include "../dependencies/flucoma-core/include/flucoma/data/FluidTensor.hpp"
#include <vector>

namespace fluid {

class VectorBufferAdaptor : public client::BufferAdaptor {
  public:
    VectorBufferAdaptor(std::vector<float> &data, index numChannels,
                        index numFrames, double sampleRate);

    bool acquire() const override;
    void release() const override;

    bool valid() const override;
    bool exists() const override;

    const client::Result resize(index frames, index channels,
                                double sampleRate) override;

    std::string asString() const override;

    FluidTensorView<float, 2> allFrames() override;
    FluidTensorView<const float, 2> allFrames() const override;

    FluidTensorView<float, 1> samps(index channel) override;
    FluidTensorView<float, 1> samps(index offset, index nframes,
                                    index chanoffset) override;

    FluidTensorView<const float, 1> samps(index channel) const override;
    FluidTensorView<const float, 1> samps(index offset, index nframes,
                                          index chanoffset) const override;

    index numFrames() const override;
    index numChans() const override;
    double sampleRate() const override;

  private:
    FluidTensorView<float, 2> mData;
    index mNumFrames;
    index mNumChannels;
    double mSampleRate;
    mutable bool mAcquired;
};

} // namespace fluid
