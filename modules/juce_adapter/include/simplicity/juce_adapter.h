#pragma once
#include <simplicity/engine.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace simplicity {
class JuceGainAdapter final {
public:
    [[nodiscard]] bool prepare(AudioSpec spec) noexcept;
    [[nodiscard]] bool setGain(float gain) noexcept;
    // All-or-nothing validation. Rejects excess channels/frames without modifying audio.
    [[nodiscard]] bool process(juce::AudioBuffer<float>& buffer) const noexcept;
private:
    GainProcessor processor_;
    AudioSpec spec_{};
    bool prepared_{false};
};
} // namespace simplicity
