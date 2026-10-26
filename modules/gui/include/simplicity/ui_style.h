#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <simplicity/ui_assets.h>

namespace simplicity::ui {
// Message-thread rendering helpers; callers own components and keep Look alive
// until every component using it has detached. Factory results transfer to JUCE.
inline const juce::Colour background{0xff121212}, panel{0xff191919}, raised{0xff242424}, border{0xff303030};
inline const juce::Colour foreground{0xffe5e5e5}, secondary{0xffa0a0a0}, selected{0xff363636}, focus{0xffc8c8c8};
inline constexpr float cornerRadius = 4.0F;
enum class Glyph { none, play, pause, stop, record, loop, undo, redo, folder, tracks,
                   grid, mixer, help, plus, minus, search, duplicate, trash, document,
                   wave, pattern, texture, rhythm, arrow, speaker, chevronDown, close, minimize, maximize };
juce::FontOptions font(float height = 14.0F, bool strong = false);
void label(juce::Graphics&, const juce::String&, juce::Rectangle<int>, float height = 14.0F,
           juce::Colour colour = foreground, bool strong = false);
void glyph(juce::Graphics&, Glyph, juce::Rectangle<float>, juce::Colour);
// Downward ruler marker; its tip aligns with the timeline playhead.
void playheadMarker(juce::Graphics&,juce::Rectangle<float>);
class IconButton final : public juce::TextButton {
public:
    IconButton(const juce::String& name, Glyph symbol, bool caption = false);
    Glyph symbol;
    bool caption;
};
class Look final : public juce::LookAndFeel_V4 {
public:
    Look();
    ~Look() override;
    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;
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
    void drawFileBrowserRow(juce::Graphics&,int,int,const juce::File&,const juce::String&,juce::Image*,const juce::String&,const juce::String&,bool,bool,int,juce::DirectoryContentsDisplayComponent&) override;
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    juce::Button* createDocumentWindowButton(int) override;
    void drawDocumentWindowTitleBar(juce::DocumentWindow&, juce::Graphics&, int, int, int, int, const juce::Image*, bool) override;
private:
    std::shared_ptr<Resources> assets_;
};
}
