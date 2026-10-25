#include <simplicity/engine.h>
#include <array>
#include <limits>
#include <random>
#include "test_support.h"
int main() {
    simplicity::GainProcessor processor;
    std::array<float, 4> samples{1, -1, 0, 0.5F};
    CHECK(!processor.process(samples));
    CHECK(samples[0] == 1);
    CHECK(processor.prepare({48000, 2, 4}));
    CHECK(processor.process(samples));
    CHECK(samples[3] == 0.5F);
    CHECK(processor.setGain(0.5F));
    CHECK(processor.process(samples));
    CHECK((samples == std::array<float, 4>{0.5F, -0.5F, 0, 0.25F}));
    for (float gain : {-1.0F, 4.01F, std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN()}) CHECK(!processor.setGain(gain));
    CHECK(processor.process(samples));
    CHECK(samples[0] == 0.25F); // invalid controls retain last accepted gain
    CHECK(processor.process({}));
    std::array<float, 5> oversized{1, 1, 1, 1, 1};
    CHECK(!processor.process(oversized));
    for (float value : oversized) CHECK(value == 1);
    CHECK(!processor.prepare({0, 2, 4}));
    CHECK(!processor.process(samples));
    CHECK(processor.prepare({48000, 2, 4}));
    CHECK(processor.setGain(0));
    CHECK(processor.process(samples));
    for (float value : samples) CHECK(value == 0);
    CHECK(processor.setGain(4));
    samples.fill(1);
    CHECK(processor.process(samples));
    for (float value : samples) CHECK(value == 4);
    // Reproducible randomized signal/gain/block-size checks; never timing-sensitive.
    std::mt19937 random(0x51A1);
    std::uniform_real_distribution<float> signal(-1, 1), gains(0, 4);
    for (int iteration = 0; iteration < 1000; ++iteration) {
        const float gain = gains(random);
        CHECK(processor.setGain(gain));
        for (auto& sample : samples) sample = signal(random);
        auto expected = samples;
        const auto size = static_cast<std::size_t>(random() % 5);
        for (std::size_t i = 0; i < size; ++i) expected[i] *= gain;
        CHECK(processor.process(std::span<float>(samples).first(size)));
        CHECK(samples == expected);
    }
}
