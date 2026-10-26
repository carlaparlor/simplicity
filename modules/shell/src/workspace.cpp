#include <simplicity/workspace.h>
#include <algorithm>
#include <cmath>
#include <utility>

namespace simplicity {
namespace {
bool validName(const std::string& name) { return !name.empty() && name.size() <= 48 && name.find_first_not_of(' ') != std::string::npos; }
}
Workspace::Workspace() { resetDemo(); undo_.clear(); }
bool Workspace::hasTrack() const noexcept { return state_.selectedTrack >= 0 && state_.selectedTrack < static_cast<int>(state_.tracks.size()); }
void Workspace::remember() {
    if (undo_.size() == 64) undo_.erase(undo_.begin());
    undo_.push_back(state_);
    redo_.clear();
}
void Workspace::newSession() { remember(); state_ = {}; stop(); }
void Workspace::resetDemo() {
    remember(); state_ = {}; state_.title = "Late hours";
    seedPatterns();
    const char* names[] = {"Drums", "Bass", "Keys", "Atmosphere"};
    for (int i = 0; i < 4; ++i) {
        SketchTrack track; track.name = names[i]; track.colour = i;
        const double length = i == 2 ? 32.0 : 16.0;
        for (double beat = i == 1 ? 16.0 : 0.0; beat < duration; beat += length) {
            if (i == 3 && beat < 32.0) continue;
            track.clips.push_back({state_.nextId++, beat, length, i+1, false});
        }
        state_.tracks.push_back(std::move(track));
    }
    state_.selectedTrack = 0; stop();
}
bool Workspace::setTitle(std::string title) {
    if (!validName(title) || title == state_.title) return false;
    remember(); state_.title = std::move(title); return true;
}
bool Workspace::setBpm(double bpm) {
    if (!std::isfinite(bpm) || bpm < 40 || bpm > 240 || bpm == state_.bpm) return false;
    remember(); state_.bpm = bpm; return true;
}
bool Workspace::addTrack(std::string name) {
    if (!validName(name) || state_.tracks.size() >= maxTracks) return false;
    remember(); SketchTrack track; track.name = std::move(name); track.colour = static_cast<int>(state_.tracks.size()) % 4;
    state_.tracks.push_back(std::move(track)); state_.selectedTrack = static_cast<int>(state_.tracks.size()) - 1; state_.selectedClip = 0; return true;
}
bool Workspace::removeTrack() {
    if (!hasTrack()) return false;
    remember(); state_.tracks.erase(state_.tracks.begin() + state_.selectedTrack);
    state_.selectedTrack = static_cast<int>(state_.tracks.size()) - 1; state_.selectedClip = 0; return true;
}
bool Workspace::selectTrack(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(state_.tracks.size())) return false;
    state_.selectedTrack = index; state_.selectedClip = 0; return true;
}
const SketchClip* Workspace::clip(int id) const noexcept {
    for (const auto& track : state_.tracks)
        for (const auto& c : track.clips) if (c.id == id) return &c;
    return nullptr;
}
bool Workspace::selectClip(int id) noexcept {
    for (std::size_t i = 0; i < state_.tracks.size(); ++i)
        for (const auto& c : state_.tracks[i].clips) if (c.id == id) {
            state_.selectedTrack = static_cast<int>(i); state_.selectedClip = id; return true;
        }
    return false;
}
bool Workspace::renameTrack(std::string name) {
    if (!hasTrack() || !validName(name) || name == state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].name) return false;
    remember(); state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].name = std::move(name); return true;
}
bool Workspace::setLevel(double level) {
    if (!hasTrack() || !std::isfinite(level) || level < -60 || level > 6) return false;
    if (state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].level == level) return false;
    remember(); state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].level = level; return true;
}
bool Workspace::setPan(double pan) {
    if(!hasTrack() || !std::isfinite(pan) || pan < -1 || pan > 1 || state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].pan==pan) return false;
    remember(); state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].pan=pan; return true;
}
bool Workspace::toggleMute() {
    if (!hasTrack()) return false;
    remember(); auto& track = state_.tracks[static_cast<std::size_t>(state_.selectedTrack)]; track.muted = !track.muted; return true;
}
bool Workspace::toggleSolo() {
    if (!hasTrack()) return false;
    remember(); auto& track = state_.tracks[static_cast<std::size_t>(state_.selectedTrack)]; track.solo = !track.solo; return true;
}
bool Workspace::addClip() {
    if (!hasTrack()) return false;
    auto& clips = state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].clips;
    if (clips.size() >= 64) return false;
    remember(); const double beat = std::clamp(snapped(position_, grid), 0.0, duration - 4);
    state_.selectedClip = state_.nextId++;
    if (state_.activePattern == 0) {
        state_.patterns.push_back({state_.nextPattern++,384,0,"Pattern 1",{}});
        state_.activePattern = state_.patterns.back().id;
    }
    clips.push_back({state_.selectedClip, beat, 4, state_.activePattern, false}); return true;
}
double Workspace::snapped(double beat, double grid) noexcept {
    if (!std::isfinite(beat)) return 0;
    if (!std::isfinite(grid) || grid <= 0) return beat;
    return std::round(beat / grid) * grid;
}
bool Workspace::moveClip(int id, double beat) {
    const auto* found = clip(id);
    if (!found || !std::isfinite(beat)) return false;
    const double target = std::clamp(snapped(beat, grid), 0.0, duration - found->length);
    if (target == found->beat) return false;
    remember();
    for (auto& track : state_.tracks) for (auto& c : track.clips) if (c.id == id) c.beat = target;
    return true;
}
bool Workspace::duplicateClip() {
    const auto* found = clip(state_.selectedClip);
    if (!found || !hasTrack()) return false;
    auto& clips = state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].clips;
    if (clips.size() >= 64) return false;
    auto copy = *found; copy.beat = std::min(copy.beat + copy.length, duration - copy.length);
    remember(); copy.id = state_.nextId++; state_.selectedClip = copy.id; clips.push_back(std::move(copy)); return true;
}
bool Workspace::removeClip() {
    if (!clip(state_.selectedClip) || !hasTrack()) return false;
    remember(); auto& clips = state_.tracks[static_cast<std::size_t>(state_.selectedTrack)].clips;
    std::erase_if(clips, [this](const auto& c) { return c.id == state_.selectedClip; }); state_.selectedClip = 0; return true;
}
bool Workspace::undo() {
    if (undo_.empty()) return false;
    redo_.push_back(state_); state_ = std::move(undo_.back()); undo_.pop_back(); return true;
}
bool Workspace::redo() {
    if (redo_.empty()) return false;
    undo_.push_back(state_); state_ = std::move(redo_.back()); redo_.pop_back(); return true;
}
void Workspace::togglePlay() noexcept { if (position_ >= transportLength()) position_ = 0; playing_ = !playing_; }
void Workspace::stop() noexcept { playing_ = false; position_ = 0; }
bool Workspace::seek(double beat) noexcept {
    if (!std::isfinite(beat)) return false;
    position_ = std::clamp(beat, 0.0, duration); return true;
}
void Workspace::advance(double seconds) noexcept {
    if (!playing_ || !std::isfinite(seconds) || seconds <= 0) return;
    position_ += std::min(seconds, 60.0) * state_.bpm / 60.0;
    const double end = transportLength();
    if (position_ >= end) {
        if (loop) position_ = std::fmod(position_, end);
        else { position_ = end; playing_ = false; }
    }
}
} // namespace simplicity
