#include <simplicity/juce_adapter.h>
#include "test_support.h"
int main() {
    simplicity::JuceGainAdapter adapter;
    juce::AudioBuffer<float> buffer(2, 4);
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < 4; ++i) buffer.setSample(ch, i, static_cast<float>(ch + 1));
    CHECK(!adapter.process(buffer));
    CHECK(buffer.getSample(0, 0) == 1);
    CHECK(adapter.prepare({48000, 2, 4}));
    CHECK(adapter.setGain(0.5F));
    CHECK(!adapter.setGain(-1));
    CHECK(adapter.process(buffer));
    CHECK(buffer.getSample(0, 0) == 0.5F);
    CHECK(buffer.getSample(1, 3) == 1);
    juce::AudioBuffer<float> tooManyChannels(3, 4);
    tooManyChannels.clear();
    tooManyChannels.setSample(0, 0, 1);
    CHECK(!adapter.process(tooManyChannels));
    CHECK(tooManyChannels.getSample(0, 0) == 1);
    juce::AudioBuffer<float> tooManyFrames(2, 5);
    tooManyFrames.clear();
    tooManyFrames.setSample(0, 0, 1);
    CHECK(!adapter.process(tooManyFrames));
    CHECK(tooManyFrames.getSample(0, 0) == 1);
    juce::AudioBuffer<float> empty(0, 0);
    CHECK(adapter.process(empty));
    CHECK(!adapter.prepare({0, 2, 4}));
    CHECK(!adapter.process(buffer));
    CHECK(adapter.prepare({48000, 2, 4}));
    CHECK(adapter.process(buffer));
}
