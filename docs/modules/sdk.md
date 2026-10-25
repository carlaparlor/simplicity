# SDK contract

## Responsibility
Describe a trusted extension using a C11-compatible V1 struct, opaque context
and process callback. Validate non-null descriptor/callback, minimum size, exact
ABI version. A larger descriptor with the same version is forward-compatible.
There is no loader, registry, ownership management or permission sandbox yet.

## Dependencies
Only standard C `<stdint.h>`; no C++, JUCE or first-party dependencies.

## Threading
The future host calls process on one audio thread with caller-owned writable
samples. Extension context remains extension-owned and must outlive callbacks.
Native code must not throw across the ABI, block, allocate or retain the buffer.
Descriptor validation requires readable memory, not an arbitrary untrusted pointer.

## Tests
`tests/sdk_test.c`: actual C consumer/callback and validation failure cases.
`tests/sdk_cpp_test.cpp`: C++ inclusion and standard-layout/trivially-copyable ABI.

## Extension points
Append fields without reordering V1; bump ABI for incompatible semantics. Future
load/unload and capability discovery need separately versioned host contracts.
Use matching platform, architecture and calling convention; no universal ABI is
promised across compilers/operating systems.
