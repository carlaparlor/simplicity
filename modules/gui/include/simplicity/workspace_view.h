#pragma once
#include <simplicity/workspace.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace simplicity {
enum class ShellAction { playPause, stop, newSession, demo, addTrack, addClip, remove,
                         duplicate, undo, redo, browser, mixer, inspector, help };
// Message-thread-only composition root. Owns UI state, never touches an audio callback.
class WorkspaceView final : public juce::Component {
public:
    WorkspaceView();
    ~WorkspaceView() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;
    void perform(ShellAction);
    [[nodiscard]] const Workspace& workspace() const noexcept;
    // Real controls/layout/render integration test; returns an error message, or empty on success.
    [[nodiscard]] juce::String runSmokeTest();
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WorkspaceView)
};
} // namespace simplicity
