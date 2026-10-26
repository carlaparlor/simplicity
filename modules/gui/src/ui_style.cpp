#include <simplicity/ui_style.h>
#include <algorithm>
#include <cmath>

namespace simplicity::ui {
void label(juce::Graphics& g, const juce::String& s, juce::Rectangle<int> r, float height, juce::Colour colour, bool strong) {
    g.setColour(colour); g.setFont(font(height, strong)); g.drawText(s, r, juce::Justification::centredLeft, true);
}
IconButton::IconButton(const juce::String& name, Glyph icon, bool showCaption) : TextButton(name), symbol(icon), caption(showCaption) { setName(name); setTooltip(name); }
Look::Look() : assets_(resources()) {
    setDefaultSansSerifTypefaceName("RetroByte");
    setColour(juce::TextEditor::backgroundColourId, background); setColour(juce::TextEditor::textColourId, foreground);
    setColour(juce::TextEditor::outlineColourId, border); setColour(juce::TextEditor::focusedOutlineColourId, focus);
    setColour(juce::TextEditor::highlightColourId, selected); setColour(juce::CaretComponent::caretColourId, foreground);
    setColour(juce::ComboBox::backgroundColourId, raised); setColour(juce::ComboBox::textColourId, foreground);
    setColour(juce::ComboBox::outlineColourId, border); setColour(juce::Label::textColourId, foreground);
    setColour(juce::PopupMenu::backgroundColourId, panel); setColour(juce::PopupMenu::textColourId, foreground);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, selected); setColour(juce::PopupMenu::highlightedTextColourId, foreground);
    setColour(juce::ScrollBar::thumbColourId, selected); setColour(juce::ResizableWindow::backgroundColourId, background);
    setColour(juce::Slider::textBoxTextColourId, foreground); setColour(juce::Slider::textBoxBackgroundColourId, panel);
    setColour(juce::Slider::textBoxHighlightColourId, selected);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack); setColour(juce::Slider::backgroundColourId, panel);
    setColour(juce::TooltipWindow::backgroundColourId, raised); setColour(juce::TooltipWindow::textColourId, foreground); setColour(juce::TooltipWindow::outlineColourId, border);
}
Look::~Look() = default;
juce::Typeface::Ptr Look::getTypefaceForFont(const juce::Font& requested) {
    if (requested.getTypefaceName()=="RetroByte" || requested.getTypefaceName()==juce::Font::getDefaultSansSerifFontName())
        return juce::Font(font(requested.getHeight())).getTypefacePtr();
    // Preserve explicit OS fallback faces for characters RetroByte does not cover.
    return juce::LookAndFeel_V4::getTypefaceForFont(requested);
}
juce::Font Look::getTextButtonFont(juce::TextButton&, int) { return juce::Font(font()); }
juce::Font Look::getComboBoxFont(juce::ComboBox&) { return juce::Font(font()); }
juce::Font Look::getPopupMenuFont() { return juce::Font(font(14)); }
juce::Label* Look::createSliderTextBox(juce::Slider& slider) {
    auto* box = juce::LookAndFeel_V4::createSliderTextBox(slider);
    box->setFont(font(14)); box->setBorderSize({0,4,0,4});
    return box;
}
void Look::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) {
    const bool primary = static_cast<bool>(b.getProperties().getWithDefault("primary", false));
    const bool flat = static_cast<bool>(b.getProperties().getWithDefault("flat", false));
    const auto r = b.getLocalBounds().toFloat().reduced(0.5F);
    const auto colour=b.getToggleState() ? (primary?foreground:selected) : (down?selected:over?raised.brighter(0.12F):raised);
    if(!flat || over || b.getToggleState()) {
        g.setColour(juce::Colours::black.withAlpha(0.5F));g.fillRoundedRectangle(r.translated(0,1),cornerRadius);
        g.setGradientFill(juce::ColourGradient(colour.brighter(down?0.0F:0.12F),0,0,colour.darker(down?0.25F:0.08F),0,r.getBottom(),false));g.fillRoundedRectangle(r,cornerRadius);
        g.setColour(border.brighter(down?0.0F:0.2F));g.drawRoundedRectangle(r,cornerRadius,0.75F);
    }
    if (b.hasKeyboardFocus(false)) { g.setColour(focus); g.drawRoundedRectangle(r.reduced(1), cornerRadius, 1); }
}
void Look::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) {
    const bool primary = b.getToggleState() && static_cast<bool>(b.getProperties().getWithDefault("primary", false));
    const auto colour = primary ? background : b.isEnabled() ? foreground : secondary.withAlpha(0.35F);
    if (auto* icon = dynamic_cast<IconButton*>(&b)) {
        if (icon->caption) {
            glyph(g, icon->symbol, {10, (static_cast<float>(b.getHeight())-18)/2, 18, 18}, colour);
            label(g, b.getButtonText(), b.getLocalBounds().withTrimmedLeft(36).reduced(0, 1),14, colour);
        } else glyph(g, icon->symbol, b.getLocalBounds().toFloat().withSizeKeepingCentre(19, 19), colour);
    } else { g.setColour(colour); g.setFont(font()); g.drawText(b.getButtonText(), b.getLocalBounds().reduced(5, 0), juce::Justification::centred, true); }
}
void Look::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) {
    g.setColour(raised); g.fillRoundedRectangle(0, 0, static_cast<float>(w), static_cast<float>(h), cornerRadius);
    if (box.hasKeyboardFocus(true)) { g.setColour(focus); g.drawRoundedRectangle(1, 1, static_cast<float>(w-2), static_cast<float>(h-2), cornerRadius, 1); }
    glyph(g, Glyph::chevronDown, {static_cast<float>(w-22), (static_cast<float>(h)-16)/2, 16, 16}, secondary);
}
void Look::fillTextEditorBackground(juce::Graphics& g, int w, int h, juce::TextEditor& e) { g.setColour(e.findColour(juce::TextEditor::backgroundColourId)); g.fillRoundedRectangle(0, 0, static_cast<float>(w), static_cast<float>(h), cornerRadius); }
void Look::drawTextEditorOutline(juce::Graphics& g, int w, int h, juce::TextEditor& e) { g.setColour(e.findColour(e.hasKeyboardFocus(true) ? juce::TextEditor::focusedOutlineColourId : juce::TextEditor::outlineColourId)); g.drawRoundedRectangle(0.5F, 0.5F, static_cast<float>(w)-1, static_cast<float>(h)-1, cornerRadius, 1); }
void Look::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle style, juce::Slider& slider) {
    if (slider.hasKeyboardFocus(false)) { g.setColour(focus); g.drawRoundedRectangle(slider.getLocalBounds().toFloat().reduced(1), cornerRadius, 1); }
    if (style == juce::Slider::LinearVertical) {
        const float cx = static_cast<float>(x+w/2); g.setColour(background); g.fillRoundedRectangle(cx-2, static_cast<float>(y), 4, static_cast<float>(h), 2);
        g.setColour(secondary); g.fillRect(cx-1, pos, 2.0F, static_cast<float>(y+h)-pos);
        g.setColour(juce::Colours::black.withAlpha(0.8F));g.fillRoundedRectangle(cx-12,pos-5,26,15,2);
        g.setGradientFill(juce::ColourGradient(foreground,cx,pos-7,secondary,cx,pos+6,false));g.fillRoundedRectangle(cx-12,pos-7,24,13,2);
        g.setColour(border);g.drawHorizontalLine(static_cast<int>(pos),cx-8,cx+8);

    } else {
        const float cy = static_cast<float>(y+h/2); g.setColour(background); g.fillRoundedRectangle(static_cast<float>(x), cy-2, static_cast<float>(w), 4, 2);
        g.setColour(secondary); g.fillRect(static_cast<float>(x), cy-1, std::max(0.0F, pos-static_cast<float>(x)), 2.0F);
        g.setColour(foreground); g.fillRoundedRectangle(pos-3, cy-7, 6, 14, 2);
    }
}
void Look::drawFileBrowserRow(juce::Graphics& g,int w,int h,const juce::File&,const juce::String& name,juce::Image*,const juce::String&,const juce::String&,bool folder,bool active,int,juce::DirectoryContentsDisplayComponent&){
    if(active)g.fillAll(selected);
    glyph(g,folder?Glyph::folder:Glyph::document,{3,static_cast<float>(h-18)/2,18,18},secondary);
    label(g,name,{27,0,w-30,h},14,active?foreground:secondary);
}
void Look::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float start,float end,juce::Slider&) {
    const auto r=juce::Rectangle<float>(static_cast<float>(x),static_cast<float>(y),static_cast<float>(w),static_cast<float>(h)).reduced(3);
    g.setColour(background);g.fillEllipse(r.translated(0,2));
    g.setGradientFill(juce::ColourGradient(selected.brighter(0.3F),r.getX(),r.getY(),raised,r.getX(),r.getBottom(),false));g.fillEllipse(r);
    g.setColour(focus);const float a=start+value*(end-start);const auto centre=r.getCentre();g.drawLine(centre.x,centre.y,centre.x+std::sin(a)*r.getWidth()*0.35F,centre.y-std::cos(a)*r.getHeight()*0.35F,2);
}
namespace {
class WindowButton final : public juce::Button {
public:
    int kind;
    explicit WindowButton(int type) : Button(type == juce::DocumentWindow::closeButton ? "Close" : type == juce::DocumentWindow::minimiseButton ? "Minimize" : "Maximize or restore"), kind(type) { setTooltip(getName()); }
    void paintButton(juce::Graphics& g, bool over, bool) override {
        g.fillAll(over ? raised : background);
        const auto symbol = kind == juce::DocumentWindow::closeButton ? Glyph::close : kind == juce::DocumentWindow::minimiseButton ? Glyph::minimize : Glyph::maximize;
        glyph(g, symbol, getLocalBounds().toFloat().withSizeKeepingCentre(14,14), foreground);
    }
};
}
juce::Button* Look::createDocumentWindowButton(int kind) { return new WindowButton(kind); }
void Look::drawDocumentWindowTitleBar(juce::DocumentWindow&, juce::Graphics& g, int width, int height, int titleX, int titleWidth, const juce::Image* icon, bool) {
    g.fillAll(background);
    if (icon != nullptr && icon->isValid()) g.drawImageWithin(*icon, titleX+4, (height-16)/2, 16, 16, juce::RectanglePlacement::centred);
    label(g, "Simplicity", {titleX+26, 0, std::min(titleWidth-26, width-titleX-26), height}, 12, secondary);
}
}
