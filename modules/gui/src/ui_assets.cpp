#include <simplicity/ui_assets.h>
#include <simplicity/ui_asset_data.h>
#include <simplicity/ui_style.h>
#include <array>

namespace simplicity::ui {
static_assert(std::size(data::lucide)==static_cast<std::size_t>(Glyph::maximize));
class Resources final {
public:
    juce::Typeface::Ptr face{juce::Typeface::createSystemTypefaceFor(data::retrobyte, sizeof(data::retrobyte))};
    std::array<juce::Path, std::size(data::lucide)> icons;
    Resources() {
        jassert(face != nullptr);
        for (std::size_t n=0; n<icons.size(); ++n) {
            const auto svg = juce::String(data::lucide[n]).replace("currentColor", "#ffffff");
            if (auto xml = juce::XmlDocument::parse(svg))
                if (auto drawable = juce::Drawable::createFromSVG(*xml)) icons[n] = drawable->getOutlineAsPath();
            jassert(!icons[n].isEmpty());
        }
    }
};
std::shared_ptr<Resources> resources() {
    static std::weak_ptr<Resources> cache;
    auto result = cache.lock();
    if (!result) { result=std::make_shared<Resources>(); cache=result; }
    return result;
}
juce::FontOptions font(float height, bool) {
    // The supplied face is Medium only; do not silently synthesize bold/another face.
    return juce::FontOptions(resources()->face).withHeight(height);
}
void glyph(juce::Graphics& g, Glyph kind, juce::Rectangle<float> bounds, juce::Colour colour) {
    const int index=static_cast<int>(kind)-1;
    if (index<0 || index>=static_cast<int>(std::size(data::lucide)) || bounds.isEmpty()) return;
    auto assets=resources(); auto& icon=assets->icons[static_cast<std::size_t>(index)];
    if (icon.isEmpty()) return;
    // Native SVG parser supplies exact stroked outlines in SVG coordinates, not
    // Drawable component ink bounds/origin. Tint only at paint time; no mutable tint.
    g.setColour(colour);
    g.fillPath(icon,juce::RectanglePlacement(juce::RectanglePlacement::centred).getTransformToFit(juce::Rectangle<float>{0,0,24,24},bounds));
}
}
