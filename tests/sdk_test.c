#include <simplicity/extension.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) return EXIT_FAILURE; } while (0)
static void process(void* context, float* samples, uint32_t frames) {
    const float gain = *(const float*)context;
    for (uint32_t i = 0; i < frames; ++i) samples[i] *= gain;
}
int main(void) {
    float gain = 0.5f;
    float samples[] = {1.0f, -1.0f};
    simplicity_extension_v1 extension = {sizeof(simplicity_extension_v1), SIMPLICITY_EXTENSION_ABI_V1, &gain, process};
    CHECK(!simplicity_extension_valid(0));
    CHECK(simplicity_extension_valid(&extension));
    extension.process(extension.context, samples, 2);
    CHECK(samples[0] == 0.5f && samples[1] == -0.5f);
    extension.abi_version = 2;
    CHECK(!simplicity_extension_valid(&extension));
    extension.abi_version = 1;
    extension.struct_size = 0;
    CHECK(!simplicity_extension_valid(&extension));
    extension.struct_size = sizeof(extension) + 16;
    CHECK(simplicity_extension_valid(&extension));
    extension.process = 0;
    CHECK(!simplicity_extension_valid(&extension));
    return EXIT_SUCCESS;
}
