#pragma once
#include <simplicity/workspace.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace simplicity {
enum class ShellAction { playPause, stop, newSession, demo, addTrack, addClip, remove,
                         duplicate, undo, redo, browser, mixer, help };
// Message-thread-only composition root. Owns UI state, never touches an audio callback.
class WorkspaceView final : public juce::Component, public juce::DragAndDropContainer, private juce::KeyListener {
public:
    WorkspaceView();
    ~WorkspaceView() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override;
    void perform(ShellAction);
    [[nodiscard]] const Workspace& workspace() const noexcept;
    // Real controls/layout/render integration test; returns an error message, or empty on success.
    [[nodiscard]] juce::String runSmokeTest();
private:
    bool keyPressed(const juce::KeyPress&, juce::Component*) override;
    class Impl;
    std::unique_ptr<Impl> impl_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkspaceView)
};
} // namespace simplicity
