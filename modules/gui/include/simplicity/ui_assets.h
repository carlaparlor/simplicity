#pragma once
#include <memory>
namespace simplicity::ui {
class Resources;
// Message-thread-only shared asset cache; Look holds it for its component tree.
// The weak registry owns nothing after the last Look/font draw call releases it.
std::shared_ptr<Resources> resources();
}
