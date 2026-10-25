#include <simplicity/extension.h>
#include <type_traits>
#include "test_support.h"
static_assert(std::is_standard_layout_v<simplicity_extension_v1>);
static_assert(std::is_trivially_copyable_v<simplicity_extension_v1>);
int main() {
    CHECK(!simplicity_extension_valid(nullptr));
    simplicity_extension_v1 descriptor{};
    CHECK(!simplicity_extension_valid(&descriptor));
}
