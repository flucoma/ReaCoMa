#include "VectorBufferAdaptor.h"

namespace fluid {

VectorBufferAdaptor::VectorBufferAdaptor(std::vector<float> &data,
                                         index numChannels, index numFrames,
                                         double sampleRate)
    : mData(data.data(), 0, numFrames, numChannels), mNumFrames(numFrames),
      mNumChannels(numChannels), mSampleRate(sampleRate), mAcquired(false) {}

bool VectorBufferAdaptor::acquire() const {
    return !mAcquired && (mAcquired = true);
}

void VectorBufferAdaptor::release() const { mAcquired = false; }

bool VectorBufferAdaptor::valid() const { return numFrames() > 0; }

bool VectorBufferAdaptor::exists() const { return true; }

const client::Result VectorBufferAdaptor::resize(index frames, index channels,
                                                 double sampleRate) {
    return client::Result{client::Result::Status::kError,
                          "Resize not supported"};
}

std::string VectorBufferAdaptor::asString() const {
    return "VectorBufferAdaptor";
}

FluidTensorView<float, 2> VectorBufferAdaptor::allFrames() { return mData; }

FluidTensorView<const float, 2> VectorBufferAdaptor::allFrames() const {
    return mData;
}

FluidTensorView<float, 1> VectorBufferAdaptor::samps(index channel) {
    return mData.col(channel);
}

FluidTensorView<float, 1>
VectorBufferAdaptor::samps(index offset, index nframes, index chanoffset) {
    return mData(Slice(offset, nframes), Slice(chanoffset, 1)).col(0);
}

FluidTensorView<const float, 1>
VectorBufferAdaptor::samps(index channel) const {
    return mData.col(channel);
}

FluidTensorView<const float, 1>
VectorBufferAdaptor::samps(index offset, index nframes,
                           index chanoffset) const {
    return mData(Slice(offset, nframes), Slice(chanoffset, 1)).col(0);
}

index VectorBufferAdaptor::numFrames() const { return mNumFrames; }

index VectorBufferAdaptor::numChans() const { return mNumChannels; }

double VectorBufferAdaptor::sampleRate() const { return mSampleRate; }

} // namespace fluid