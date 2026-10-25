# JUCE adapter contract

## Responsibility
Map juce::AudioBuffer<float> planar channels onto GainProcessor. Prepare first;
reject excess channels/frames before any writes. Zero-size/channel buffers are
valid no-ops after preparation. No device management or GUI behavior.

## Dependencies
Direct `engine` dependency plus pinned `juce::juce_audio_basics` (transitively
juce_core). Core types appear through the engine public interface, not a separate
undeclared direct include. JUCE never leaks into the engine/core/SDK.

## Threading
Same stopped-configuration/single-audio-thread contract as the engine. Caller
owns buffer memory. Processing has no allocations, locks, logging or exceptions.
Constructing/resizing AudioBuffer is performed outside processing.

## Tests
`tests/juce_adapter_test.cpp`: actual JUCE buffers, channels, empty and oversized
blocks, gain failures and preparation recovery. `juce_bootstrap` links/runs the
headless application. Not a mock JUCE replacement.

## Extension points
Keep future device callbacks, plugin hosting and GUI adaptation in focused edge
modules. Do not hide session state or cross-thread control here.
