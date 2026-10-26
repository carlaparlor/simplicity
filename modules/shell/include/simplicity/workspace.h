#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace simplicity {
// UI-only session sketch. No audio, media ownership, persistence or audio-thread use.
struct PatternNote {
    int tick{}, length{24}, pitch{60}, channel{3}, velocity{100};
    bool operator==(const PatternNote&) const = default;
};
struct Pattern {
    int id{}, length{384}, colour{};
    std::string name;
    std::vector<PatternNote> notes;
};
struct SketchClip {
    int id{};
    double beat{}, length{};
    int pattern{};
    bool muted{};
    double offset{};
};
struct SketchTrack {
    std::string name;
    int colour{};
    bool muted{}, solo{};
    double level{-6.0}, pan{};
    std::vector<SketchClip> clips;
};
class Workspace final {
public:
    static constexpr double duration = 64.0;
    static constexpr int maxTracks = 24, maxPatterns = 128, ticksPerBeat = 96, maxNotes = 512;
    [[nodiscard]] const std::vector<Pattern>& patterns() const noexcept { return state_.patterns; }
    [[nodiscard]] const Pattern* pattern(int id) const noexcept;
    [[nodiscard]] int activePattern() const noexcept { return state_.activePattern; }
    bool selectPattern(int id) noexcept;
    bool addPattern(std::string name, int beats = 4);
    bool renamePattern(std::string name);
    bool clonePattern();
    bool deletePattern(); // Reject in-use sources, without modifying history.
    bool makeUnique();
    bool addNote(PatternNote note);
    bool removeNote(int index);
    bool resizeNote(int index,int length);
    bool paintPattern(int lane, const std::vector<double>& beats); // Atomic, deduplicated stroke.
    bool placePattern(int lane, double beat);
    bool clonePlacement(int id, int lane, double beat);
    bool resizeClip(int id, double beats);
    bool sliceClip(int id, double beat);
    bool toggleClipMute(int id);
    bool moveClipTo(int id, int lane, double beat, bool useSnap = true);
    bool patternMode{}; // Visual transport only, not undo state.
    void deselectClip() noexcept { state_.selectedClip = 0; }
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
    bool setPan(double pan);
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
        std::vector<Pattern> patterns;
        int activePattern{}, nextPattern{1};
        std::string title{"Untitled session"};
        double bpm{120.0};
        int selectedTrack{-1}, selectedClip{}, nextId{1};
    } state_;
    std::vector<State> undo_, redo_;
    bool playing_{};
    double position_{};
    void remember();
    void seedPatterns();
    [[nodiscard]] double transportLength() const noexcept;
    [[nodiscard]] bool hasTrack() const noexcept;
};
} // namespace simplicity
