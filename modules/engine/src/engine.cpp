#include <simplicity/engine.h>
#include <cmath>

namespace simplicity {
bool GainProcessor::prepare(AudioSpec spec) noexcept {
    // Failed preparation invalidates the old configuration rather than silently reusing it.
    prepared_ = spec.valid();
    if (prepared_) spec_ = spec;
    return prepared_;
}
bool GainProcessor::setGain(float gain) noexcept {
    if (!std::isfinite(gain) || gain < 0.0F || gain > 4.0F) return false;
    gain_ = gain;
    return true;
}
bool GainProcessor::process(std::span<float> samples) const noexcept {
    if (!prepared_ || samples.size() > spec_.maxFrames) return false;
    for (auto& sample : samples) sample *= gain_;
    return true;
}
} // namespace simplicity
