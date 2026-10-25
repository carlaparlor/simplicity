#ifndef SIMPLICITY_EXTENSION_H
#define SIMPLICITY_EXTENSION_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define SIMPLICITY_EXTENSION_ABI_V1 1u
/* In-process, trusted extensions only. No C++ types, ownership transfer or exceptions.
 * Host and extension must share platform/architecture/calling convention.
 * process replaces planar samples in place, performs no allocation or blocking,
 * and is called by one audio thread. Lifetime belongs to the extension.
 * This is a descriptor/validation foundation, NOT a loader or sandbox. */
typedef struct simplicity_extension_v1 {
    uint32_t struct_size;
    uint32_t abi_version;
    void* context;
    void (*process)(void* context, float* samples, uint32_t frames);
} simplicity_extension_v1;
static inline int simplicity_extension_valid(const simplicity_extension_v1* extension) {
    return extension != 0
        && extension->struct_size >= sizeof(simplicity_extension_v1)
        && extension->abi_version == SIMPLICITY_EXTENSION_ABI_V1
        && extension->process != 0;
}
#ifdef __cplusplus
}
#endif
#endif
