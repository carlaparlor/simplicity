#pragma once
#include <cmath>
#include <cstdint>

namespace simplicity {
// Immutable after preparation. Maximum block size is a capacity, not a fixed block size.
struct AudioSpec {
    double sampleRate{48000.0};
    std::uint32_t channels{2};
    std::uint32_t maxFrames{512};

    [[nodiscard]] bool valid() const noexcept {
        return std::isfinite(sampleRate) && sampleRate >= 8000.0 && sampleRate <= 384000.0
            && channels > 0 && channels <= 64 && maxFrames > 0 && maxFrames <= 1048576;
    }
};
} // namespace simplicity
