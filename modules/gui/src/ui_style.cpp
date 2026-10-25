#include <simplicity/ui_style.h>
#include <algorithm>

namespace simplicity::ui {
juce::FontOptions font(float height, bool strong) { return juce::FontOptions("Segoe UI", height, strong ? juce::Font::bold : juce::Font::plain); }
void label(juce::Graphics& g, const juce::String& s, juce::Rectangle<int> r, float height, juce::Colour colour, bool strong) {
    g.setColour(colour); g.setFont(font(height, strong)); g.drawText(s, r, juce::Justification::centredLeft, true);
}
void glyph(juce::Graphics& g, Glyph kind, juce::Rectangle<float> bounds, juce::Colour colour) {
    juce::Path p;
    const auto segment = [&p](float x, float y, float xx, float yy) { p.startNewSubPath(x, y); p.lineTo(xx, yy); };
    bool filled = false;
    switch (kind) {
        case Glyph::play: p.addTriangle(8, 5, 19, 12, 8, 19); filled = true; break;
        case Glyph::pause: p.addRoundedRectangle(6, 5, 4, 14, 1); p.addRoundedRectangle(14, 5, 4, 14, 1); filled = true; break;
        case Glyph::stop: p.addRoundedRectangle(6, 6, 12, 12, 1.5F); filled = true; break;
        case Glyph::record: p.addEllipse(6, 6, 12, 12); break;
        case Glyph::loop: p.startNewSubPath(18, 8); p.cubicTo(9, 1, 1, 10, 6, 15); segment(18, 8, 18, 3); segment(18, 8, 13, 8); p.startNewSubPath(6, 16); p.cubicTo(15, 23, 23, 14, 18, 9); segment(6, 16, 6, 21); segment(6, 16, 11, 16); break;
        case Glyph::undo: p.startNewSubPath(5, 8); p.cubicTo(22, 2, 22, 21, 9, 18); segment(5, 8, 5, 3); segment(5, 8, 10, 10); break;
        case Glyph::redo: p.startNewSubPath(19, 8); p.cubicTo(2, 2, 2, 21, 15, 18); segment(19, 8, 19, 3); segment(19, 8, 14, 10); break;
        case Glyph::folder: p.startNewSubPath(3, 6); p.lineTo(10, 6); p.lineTo(12, 9); p.lineTo(21, 9); p.lineTo(21, 19); p.lineTo(3, 19); p.closeSubPath(); break;
        case Glyph::tracks: p.addRoundedRectangle(3, 4, 18, 16, 1); segment(9, 4, 9, 20); segment(3, 12, 21, 12); break;
        case Glyph::grid: for (float x : {4.0F, 14.0F}) for (float y : {4.0F, 14.0F}) p.addRoundedRectangle(x, y, 6, 6, 1); break;
        case Glyph::mixer: segment(5, 4, 5, 20); segment(12, 4, 12, 20); segment(19, 4, 19, 20); p.addRectangle(2, 7, 6, 4); p.addRectangle(9, 13, 6, 4); p.addRectangle(16, 6, 6, 4); break;
        case Glyph::help: p.addEllipse(3, 3, 18, 18); p.startNewSubPath(9, 9); p.cubicTo(9, 4, 18, 6, 13, 11); p.lineTo(12, 13); segment(12, 16, 12, 16.5F); break;
        case Glyph::plus: segment(5, 12, 19, 12); segment(12, 5, 12, 19); break;
        case Glyph::minus: segment(5, 12, 19, 12); break;
        case Glyph::search: p.addEllipse(4, 3, 12, 12); segment(14, 14, 21, 21); break;
        case Glyph::duplicate: p.addRoundedRectangle(8, 8, 12, 13, 1); p.startNewSubPath(15, 5); p.lineTo(15, 3); p.lineTo(3, 3); p.lineTo(3, 16); p.lineTo(5, 16); break;
        case Glyph::trash: segment(4, 6, 20, 6); p.addRoundedRectangle(6, 6, 12, 15, 1); segment(9, 3, 15, 3); segment(10, 10, 10, 17); segment(14, 10, 14, 17); break;
        case Glyph::document: p.startNewSubPath(5, 3); p.lineTo(14, 3); p.lineTo(19, 8); p.lineTo(19, 21); p.lineTo(5, 21); p.closeSubPath(); segment(14, 3, 14, 8); segment(14, 8, 19, 8); break;
        case Glyph::wave: for (int n = 0; n < 5; ++n) { const float x = static_cast<float>(4 + n * 4); const float h = static_cast<float>(3 + (n * 7) % 8); segment(x, 12-h, x, 12+h); } break;
        case Glyph::pattern: p.addRoundedRectangle(3, 4, 18, 16, 1); segment(7, 8, 11, 8); segment(13, 12, 18, 12); segment(7, 16, 10, 16); break;
        case Glyph::texture: for (int n = 0; n < 3; ++n) { const float y = static_cast<float>(6 + n * 6); p.startNewSubPath(3, y); p.cubicTo(9, y-6, 15, y+6, 21, y); } break;
        case Glyph::rhythm: p.addEllipse(4, 5, 16, 14); segment(5, 2, 10, 8); segment(19, 2, 14, 8); break;
        case Glyph::arrow: segment(4, 12, 20, 12); segment(14, 6, 20, 12); segment(14, 18, 20, 12); break;
        case Glyph::speaker: p.startNewSubPath(4, 9); p.lineTo(8, 9); p.lineTo(13, 5); p.lineTo(13, 19); p.lineTo(8, 15); p.lineTo(4, 15); p.closeSubPath(); p.startNewSubPath(17, 8); p.quadraticTo(23, 12, 17, 16); break;
        case Glyph::none: return;
    }
    const auto area = bounds.withSizeKeepingCentre(std::min(bounds.getWidth(), bounds.getHeight()), std::min(bounds.getWidth(), bounds.getHeight()));
    const auto transform = juce::AffineTransform::scale(area.getWidth() / 24.0F).translated(area.getX(), area.getY());
    g.setColour(colour);
    if (filled) g.fillPath(p, transform);
    else g.strokePath(p, juce::PathStrokeType(1.65F, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), transform);
}
IconButton::IconButton(const juce::String& name, Glyph icon, bool showCaption) : TextButton(name), symbol(icon), caption(showCaption) { setName(name); setTooltip(name); }
Look::Look() {
    setDefaultSansSerifTypefaceName("Segoe UI");
    setColour(juce::TextEditor::backgroundColourId, background); setColour(juce::TextEditor::textColourId, foreground);
    setColour(juce::TextEditor::outlineColourId, border); setColour(juce::TextEditor::focusedOutlineColourId, focus);
    setColour(juce::TextEditor::highlightColourId, selected); setColour(juce::CaretComponent::caretColourId, foreground);
    setColour(juce::ComboBox::backgroundColourId, raised); setColour(juce::ComboBox::textColourId, foreground);
    setColour(juce::ComboBox::outlineColourId, border); setColour(juce::Label::textColourId, foreground);
    setColour(juce::PopupMenu::backgroundColourId, panel); setColour(juce::PopupMenu::textColourId, foreground);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, selected); setColour(juce::PopupMenu::highlightedTextColourId, foreground);
    setColour(juce::ScrollBar::thumbColourId, selected); setColour(juce::ResizableWindow::backgroundColourId, background);
    setColour(juce::Slider::textBoxTextColourId, foreground); setColour(juce::Slider::textBoxBackgroundColourId, panel);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack); setColour(juce::Slider::backgroundColourId, panel);
    setColour(juce::TooltipWindow::backgroundColourId, raised); setColour(juce::TooltipWindow::textColourId, foreground); setColour(juce::TooltipWindow::outlineColourId, border);
}
juce::Font Look::getTextButtonFont(juce::TextButton&, int) { return juce::Font(font()); }
juce::Font Look::getComboBoxFont(juce::ComboBox&) { return juce::Font(font()); }
juce::Font Look::getPopupMenuFont() { return juce::Font(font(14)); }
void Look::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) {
    const bool primary = static_cast<bool>(b.getProperties().getWithDefault("primary", false));
    const bool flat = static_cast<bool>(b.getProperties().getWithDefault("flat", false));
    const auto r = b.getLocalBounds().toFloat().reduced(0.5F);
    g.setColour(b.getToggleState() ? (primary ? foreground : selected) : (down ? selected : over ? raised.brighter(0.12F) : flat ? juce::Colours::transparentBlack : raised));
    g.fillRoundedRectangle(r, cornerRadius);
    if (b.hasKeyboardFocus(false)) { g.setColour(focus); g.drawRoundedRectangle(r.reduced(1), cornerRadius, 1); }
}
void Look::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) {
    const bool primary = b.getToggleState() && static_cast<bool>(b.getProperties().getWithDefault("primary", false));
    const auto colour = primary ? background : b.isEnabled() ? foreground : secondary.withAlpha(0.35F);
    if (auto* icon = dynamic_cast<IconButton*>(&b)) {
        if (icon->caption) {
            glyph(g, icon->symbol, {10, (static_cast<float>(b.getHeight())-18)/2, 18, 18}, colour);
            label(g, b.getButtonText(), b.getLocalBounds().withTrimmedLeft(36).reduced(0, 1), 13, colour);
        } else glyph(g, icon->symbol, b.getLocalBounds().toFloat().withSizeKeepingCentre(19, 19), colour);
    } else { g.setColour(colour); g.setFont(font()); g.drawText(b.getButtonText(), b.getLocalBounds().reduced(5, 0), juce::Justification::centred, true); }
}
void Look::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) {
    g.setColour(raised); g.fillRoundedRectangle(0, 0, static_cast<float>(w), static_cast<float>(h), cornerRadius);
    if (box.hasKeyboardFocus(true)) { g.setColour(focus); g.drawRoundedRectangle(1, 1, static_cast<float>(w-2), static_cast<float>(h-2), cornerRadius, 1); }
    g.setColour(secondary); const float x = static_cast<float>(w-13), y = static_cast<float>(h)/2;
    g.drawLine(x-3, y-1, x, y+2, 1.2F); g.drawLine(x, y+2, x+3, y-1, 1.2F);
}
void Look::fillTextEditorBackground(juce::Graphics& g, int w, int h, juce::TextEditor& e) { g.setColour(e.findColour(juce::TextEditor::backgroundColourId)); g.fillRoundedRectangle(0, 0, static_cast<float>(w), static_cast<float>(h), cornerRadius); }
void Look::drawTextEditorOutline(juce::Graphics& g, int w, int h, juce::TextEditor& e) { g.setColour(e.findColour(e.hasKeyboardFocus(true) ? juce::TextEditor::focusedOutlineColourId : juce::TextEditor::outlineColourId)); g.drawRoundedRectangle(0.5F, 0.5F, static_cast<float>(w)-1, static_cast<float>(h)-1, cornerRadius, 1); }
void Look::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle style, juce::Slider& slider) {
    if (slider.hasKeyboardFocus(false)) { g.setColour(focus); g.drawRoundedRectangle(slider.getLocalBounds().toFloat().reduced(1), cornerRadius, 1); }
    if (style == juce::Slider::LinearVertical) {
        const float cx = static_cast<float>(x+w/2); g.setColour(background); g.fillRoundedRectangle(cx-2, static_cast<float>(y), 4, static_cast<float>(h), 2);
        g.setColour(secondary); g.fillRect(cx-1, pos, 2.0F, static_cast<float>(y+h)-pos);
        g.setColour(foreground); g.fillRoundedRectangle(cx-10, pos-3, 20, 6, 2);
    } else {
        const float cy = static_cast<float>(y+h/2); g.setColour(background); g.fillRoundedRectangle(static_cast<float>(x), cy-2, static_cast<float>(w), 4, 2);
        g.setColour(secondary); g.fillRect(static_cast<float>(x), cy-1, std::max(0.0F, pos-static_cast<float>(x)), 2.0F);
        g.setColour(foreground); g.fillRoundedRectangle(pos-3, cy-7, 6, 14, 2);
    }
}
namespace {
class WindowButton final : public juce::Button {
public:
    int kind;
    explicit WindowButton(int type) : Button(type == juce::DocumentWindow::closeButton ? "Close" : type == juce::DocumentWindow::minimiseButton ? "Minimize" : "Maximize or restore"), kind(type) { setTooltip(getName()); }
    void paintButton(juce::Graphics& g, bool over, bool) override {
        g.fillAll(over ? raised : background); g.setColour(foreground);
        const float x = static_cast<float>(getWidth())/2, y = static_cast<float>(getHeight())/2;
        if (kind == juce::DocumentWindow::closeButton) { g.drawLine(x-4,y-4,x+4,y+4); g.drawLine(x-4,y+4,x+4,y-4); }
        else if (kind == juce::DocumentWindow::minimiseButton) g.drawLine(x-4,y+3,x+4,y+3);
        else g.drawRoundedRectangle(x-4,y-4,8,8,1,1);
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
