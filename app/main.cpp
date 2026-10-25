#include <simplicity/juce_adapter.h>
#include <iostream>

int main() {
    simplicity::JuceGainAdapter adapter;
    juce::AudioBuffer<float> buffer(2, 128);
    buffer.clear();
    if (!adapter.prepare({48000.0, 2, 128}) || !adapter.process(buffer)) return 1;
    std::cout << "Simplicity: C++/JUCE engine bootstrap ready (no DAW UI yet).\n";
}
