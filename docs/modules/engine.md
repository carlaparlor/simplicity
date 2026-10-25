# Engine contract

## Responsibility
Prepare and process a single planar float channel with a finite [0,4] gain.
An unprepared or oversized block returns false without writes; empty prepared
blocks succeed. Failed preparation invalidates the previous configuration;
failed gain changes preserve the last gain. Successful reprepare preserves gain.
Input samples are not clipped, normalized or checked for non-finite values.

## Dependencies
`core` and C++ standard library only. No JUCE, devices, files or GUI.

## Threading
One owner on the audio thread. Prepare and setters are invoked while processing
is stopped. Processing is noexcept, allocation-free and non-blocking. The caller
owns a valid sample span for the duration of the call; no samples are retained.

## Tests
`tests/engine_test.cpp`: states, errors, bounds, gain/silence, unchanged rejected
buffers and 1,000 deterministic randomized gain/signal/block cases.

## Extension points
Future graph, transport and DSP components should have small explicit contracts.
Do not turn GainProcessor into a general session/controller singleton. Runtime
parameter automation will need a separate bounded, thread-safe delivery design.
