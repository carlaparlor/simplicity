#pragma once
#include <simplicity/core.h>
#include <span>

namespace simplicity {
// Single audio-thread owner. Prepare/change gain only while processing is stopped.
class GainProcessor final {
public:
    [[nodiscard]] bool prepare(AudioSpec spec) noexcept;
    [[nodiscard]] bool setGain(float gain) noexcept;
    // One planar channel at a time. Failure leaves the samples unchanged.
    [[nodiscard]] bool process(std::span<float> samples) const noexcept;
private:
    AudioSpec spec_{};
    float gain_{1.0F};
    bool prepared_{false};
};
} // namespace simplicity
