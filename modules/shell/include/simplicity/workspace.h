#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace simplicity {
// UI-only session sketch. No audio, media ownership, persistence or audio-thread use.
struct SketchClip {
    int id{};
    std::string name;
    double beat{}, length{};
};
struct SketchTrack {
    std::string name;
    int colour{};
    bool muted{}, solo{};
    double level{-6.0};
    std::vector<SketchClip> clips;
};
class Workspace final {
public:
    static constexpr double duration = 64.0;
    static constexpr int maxTracks = 24;
    Workspace();
    [[nodiscard]] const std::vector<SketchTrack>& tracks() const noexcept { return state_.tracks; }
    [[nodiscard]] const std::string& title() const noexcept { return state_.title; }
    [[nodiscard]] double bpm() const noexcept { return state_.bpm; }
    [[nodiscard]] int selectedTrack() const noexcept { return state_.selectedTrack; }
    [[nodiscard]] int selectedClip() const noexcept { return state_.selectedClip; }
    [[nodiscard]] const SketchClip* clip(int id) const noexcept;
    [[nodiscard]] bool canUndo() const noexcept { return !undo_.empty(); }
    [[nodiscard]] bool canRedo() const noexcept { return !redo_.empty(); }
    [[nodiscard]] bool playing() const noexcept { return playing_; }
    [[nodiscard]] double position() const noexcept { return position_; }
    bool loop{true}; // UI-thread only; visual loop spans the 16-bar sketch.
    double grid{1.0}; // Beats; zero disables snap.
    void togglePlay() noexcept;
    void stop() noexcept;
    void advance(double seconds) noexcept;
    bool seek(double beat) noexcept;
    void newSession();
    void resetDemo();
    bool setTitle(std::string title);
    bool setBpm(double bpm);
    bool addTrack(std::string name);
    bool removeTrack();
    bool selectTrack(int index) noexcept;
    bool selectClip(int id) noexcept;
    bool renameTrack(std::string name);
    bool setLevel(double level);
    bool toggleMute();
    bool toggleSolo();
    bool addClip();
    bool moveClip(int id, double beat);
    bool duplicateClip();
    bool removeClip();
    bool undo();
    bool redo();
    [[nodiscard]] static double snapped(double beat, double grid) noexcept;
private:
    struct State {
        std::vector<SketchTrack> tracks;
        std::string title{"Untitled session"};
        double bpm{120.0};
        int selectedTrack{-1}, selectedClip{}, nextId{1};
    } state_;
    std::vector<State> undo_, redo_;
    bool playing_{};
    double position_{};
    void remember();
    [[nodiscard]] bool hasTrack() const noexcept;
};
} // namespace simplicity
