# Core contract

## Responsibility
Own JUCE-independent audio format/capacity values. AudioSpec is a trivial value
object; supported rates are 8–384 kHz, 1–64 channels and 1–1,048,576 frames.

## Dependencies
C++ standard library only. No engine, SDK, adapter, devices or GUI dependency.

## Threading
Values are copied before processing. Concurrent mutation/read is not supported.
Validation is noexcept and performs no allocation, locking or I/O.

## Tests
`tests/core_test.cpp`: inclusive bounds, invalid rates including NaN/infinity,
zero and excessive channel/frame capacities. Runs in Debug and Release.

## Extension points
Add reusable value types here only when multiple domains need them. Keep UI,
persistence schemas and device handles out. No stable C++ binary ABI promised.
