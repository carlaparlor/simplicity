#pragma once
#include <simplicity/workspace.h>
#include <simplicity/ui_style.h>
#include <functional>

namespace simplicity {
// Message-thread-only drawing of actual notes, shared by picker and playlist.
void drawPatternNotes(juce::Graphics&, const Pattern&, juce::Rectangle<float>,
                      double offset, double beats, juce::Colour);
class PatternPanel final : public juce::Component, private juce::ListBoxModel {
public:
    explicit PatternPanel(Workspace&);
    std::function<void()> changed, edit;
    void refresh();
    void beginRename();
    void endRename(){name.hideEditor(true);}
    void resized() override;
    void paint(juce::Graphics&) override;
private:
    Workspace& model;
    juce::ListBox list{"Patterns",this};
    juce::Label name;
    ui::IconButton add{"New pattern",ui::Glyph::plus}, clone{"Clone source",ui::Glyph::duplicate}, unique{"Make unique",ui::Glyph::pattern}, remove{"Delete unused source",ui::Glyph::trash};
    juce::var getDragSourceDescription(const juce::SparseSet<int>&) override;
    int getNumRows() override;
    void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
    void selectedRowsChanged(int) override;
    void listBoxItemDoubleClicked(int,const juce::MouseEvent&) override;
};
// Borrowed Workspace outlives editor. Four named MIDI slots, not hosted plugins.
class PatternEditor final : public juce::Component {
public:
    explicit PatternEditor(Workspace&);
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
private:
    Workspace& model;
    juce::ComboBox channel;
    juce::Rectangle<int> gridBounds() const;
    int held{-1}, lastStep{-1};
    bool stretching{};
    int heldNote{-1}, gesturePattern{}, shownPattern{};
};
}
