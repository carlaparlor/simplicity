#include <simplicity/juce_adapter.h>

namespace simplicity {
bool JuceGainAdapter::prepare(AudioSpec spec) noexcept {
    prepared_ = processor_.prepare(spec);
    if (prepared_) spec_ = spec;
    return prepared_;
}
bool JuceGainAdapter::setGain(float gain) noexcept { return processor_.setGain(gain); }
bool JuceGainAdapter::process(juce::AudioBuffer<float>& buffer) const noexcept {
    if (!prepared_ || static_cast<unsigned>(buffer.getNumChannels()) > spec_.channels
        || static_cast<unsigned>(buffer.getNumSamples()) > spec_.maxFrames) return false;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        if (!processor_.process({buffer.getWritePointer(channel),
                                static_cast<std::size_t>(buffer.getNumSamples())})) return false;
    }
    return true;
}
} // namespace simplicity
