#pragma once
#include <simplicity/ui_style.h>
#include <functional>

namespace simplicity {
// Message-thread UI with worker-owned asynchronous directory enumeration. Filter
// outlives directory list; worker is joined before list/tree/filter destruction.
class BrowserPanel final : public juce::Component, private juce::FileFilter,
                           private juce::FileBrowserListener {
public:
    BrowserPanel();
    ~BrowserPanel() override;
    void refresh();
    void setQuery(const juce::String&);
    void setCategory(int);
    [[nodiscard]] int visibleFileCount() const { return directory.getNumFiles(); }
    [[nodiscard]] bool isScanning() const { return directory.isStillLoading(); }
    bool setRootDirectory(const juce::File&);
    [[nodiscard]] const juce::File& rootDirectory() const noexcept { return directory.getDirectory(); }
    [[nodiscard]] bool acceptsFile(const juce::File&) const;
    void paint(juce::Graphics&) override;
    void resized() override;
    class SearchField final : public juce::TextEditor {
        void paintOverChildren(juce::Graphics& g) override { juce::TextEditor::paintOverChildren(g); ui::glyph(g,ui::Glyph::search,{9,8,17,17},ui::secondary); }
    };
    SearchField search;
private:
    mutable juce::CriticalSection filterLock;
    juce::String query;
    int category{1};
    juce::TimeSliceThread worker{"Library directory scan"};
    juce::DirectoryContentsList directory{this,worker};
    juce::FileTreeComponent tree{directory};
    juce::ComboBox type;
    juce::Label path, details;
    ui::IconButton documents{"Documents",ui::Glyph::folder,true}, home{"Home",ui::Glyph::folder,true}, up{"Up",ui::Glyph::arrow}, add{"Add folder",ui::Glyph::plus,true};
    std::unique_ptr<juce::FileChooser> chooser;
    bool isFileSuitable(const juce::File&) const override;
    bool isDirectorySuitable(const juce::File&) const override;
    void selectionChanged() override;
    void fileClicked(const juce::File&,const juce::MouseEvent&) override {}
    void fileDoubleClicked(const juce::File&) override;
    void browserRootChanged(const juce::File&) override {}
};
}
