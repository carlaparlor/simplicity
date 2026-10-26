#pragma once
#include <simplicity/workspace.h>
#include <simplicity/ui_style.h>
#include <simplicity/zoom_controls.h>
#include <functional>

namespace simplicity {
// Message-thread-only drawing of actual notes, shared by picker and playlist.
void drawPatternNotes(juce::Graphics&, const Pattern&, juce::Rectangle<float>,
                      double offset, double beats, juce::Colour);
void drawPatternClip(juce::Graphics&,const Pattern&,juce::Rectangle<float>,double offset,double beats,bool selected,bool quiet=false);
juce::Colour patternColour(int index);
class PatternPanel final : public juce::Component {
public:
    explicit PatternPanel(Workspace&);
    ~PatternPanel() override;
    juce::Rectangle<int> cardBounds(int index) const;
    std::function<void()> changed, edit;
    void refresh();
    void beginRename();
    void endRename(){name.hideEditor(true);}
    void resized() override;
    void paint(juce::Graphics&) override;
private:
    Workspace& model;
    class Card;
    juce::Viewport list;
    juce::Component content;
    juce::OwnedArray<Card> cards;
    juce::Label name;
    ui::IconButton add{"New pattern",ui::Glyph::plus}, clone{"Clone source",ui::Glyph::duplicate}, unique{"Make unique",ui::Glyph::pattern}, remove{"Delete unused source",ui::Glyph::trash};

};
// Borrowed Workspace outlives editor. Four named MIDI slots, not hosted plugins.
class PatternEditor final : public juce::Component, private juce::ScrollBar::Listener {
public:
    explicit PatternEditor(Workspace&);
    ~PatternEditor() override;
    std::function<void()> changed;
    bool piano{};
    int basePitch{48};
    void refresh();
    void resized() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
    bool editCell(int step,int row,bool erase=false);
    bool keyPressed(const juce::KeyPress&) override;
    void zoomHorizontal(double factor,double anchor=0.5);
    void zoomVertical(double factor);
    double horizontalSpan() const {return timeZoom.span();}
    int visibleRows() const {return piano?pitchRows:4;}
    juce::Rectangle<int> gridBounds() const;
private:
    Workspace& model;
    juce::ComboBox channel;
    TimelineZoomBar timeZoom;
    VerticalZoomGrip verticalZoom;
    juce::ScrollBar pitchScroll{true};
    int pitchRows{12};
    bool panning{};
    double panStart{};
    int panPitch{};
    int stepAt(int x) const;
    void syncPitchScroll();
    void scrollBarMoved(juce::ScrollBar*,double) override;
    int held{-1}, lastStep{-1};
    bool stretching{};
    int heldNote{-1}, gesturePattern{}, shownPattern{};
};
}
