#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace simplicity::ui {
// Message-thread rendering helpers; callers own components and keep Look alive
// until every component using it has detached. Factory results transfer to JUCE.
inline const juce::Colour background{0xff181818}, panel{0xff202020}, raised{0xff292929}, border{0xff363636};
inline const juce::Colour foreground{0xffe5e5e5}, secondary{0xffa0a0a0}, selected{0xff414141}, focus{0xffc8c8c8};
inline constexpr float cornerRadius = 4.0F;
enum class Glyph { none, play, pause, stop, record, loop, undo, redo, folder, tracks,
                   grid, mixer, help, plus, minus, search, duplicate, trash, document,
                   wave, pattern, texture, rhythm, arrow, speaker };
juce::FontOptions font(float height = 13.0F, bool strong = false);
void label(juce::Graphics&, const juce::String&, juce::Rectangle<int>, float height = 13.0F,
           juce::Colour colour = foreground, bool strong = false);
void glyph(juce::Graphics&, Glyph, juce::Rectangle<float>, juce::Colour);
class IconButton final : public juce::TextButton {
public:
    IconButton(const juce::String& name, Glyph symbol, bool caption = false);
    Glyph symbol;
    bool caption;
};
class Look final : public juce::LookAndFeel_V4 {
public:
    Look();
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Label* createSliderTextBox(juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    void fillTextEditorBackground(juce::Graphics&, int, int, juce::TextEditor&) override;
    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    juce::Button* createDocumentWindowButton(int) override;
    void drawDocumentWindowTitleBar(juce::DocumentWindow&, juce::Graphics&, int, int, int, int, const juce::Image*, bool) override;
};
}
