#include <simplicity/core.h>
#include <limits>
#include "test_support.h"
int main() {
    using simplicity::AudioSpec;
    CHECK(AudioSpec{}.valid());
    for (double rate : {8000.0, 44100.0, 48000.0, 384000.0}) CHECK((AudioSpec{rate, 1, 1}.valid()));
    for (double rate : {0.0, -1.0, 7999.0, 384001.0,
                        std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
        CHECK((!AudioSpec{rate, 2, 512}.valid()));
    CHECK((AudioSpec{48000, 64, 1048576}.valid()));
    CHECK((!AudioSpec{48000, 0, 512}.valid()));
    CHECK((!AudioSpec{48000, 65, 512}.valid()));
    CHECK((!AudioSpec{48000, 2, 0}.valid()));
    CHECK((!AudioSpec{48000, 2, 1048577}.valid()));
}
