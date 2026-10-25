#pragma once
#include <simplicity/workspace.h>
#include <simplicity/ui_style.h>
#include <functional>

namespace simplicity {
// Message-thread-only navigator for actual in-memory session tracks and usable templates.
// No filesystem, sample/instrument catalogue or plugin discovery is implied.
class BrowserPanel final : public juce::Component {
public:
    explicit BrowserPanel(Workspace&);
    ~BrowserPanel() override;
    std::function<void()> changed;
    void refresh();
    void paint(juce::Graphics&) override;
    void resized() override;
    void setTemplateTab(bool);
    void setQuery(const juce::String&);
    [[nodiscard]] int resultCount() const noexcept { return static_cast<int>(results_.size()); }
    bool activateResult(int index);
    class SearchField final : public juce::TextEditor {
        void paintOverChildren(juce::Graphics& g) override { juce::TextEditor::paintOverChildren(g); ui::glyph(g, ui::Glyph::search, {9, 8, 17, 17}, ui::secondary); }
    };
    SearchField search;
private:
    Workspace& model_;
    bool templates_{};
    ui::IconButton session_{"Session", ui::Glyph::tracks, true}, templatesButton_{"Templates", ui::Glyph::grid, true}, add_{"New track", ui::Glyph::plus, true};
    juce::Component content_;
    juce::Viewport viewport_;
    juce::OwnedArray<ui::IconButton> rows_;
    std::vector<int> results_;
    void updateResults();
};
}
