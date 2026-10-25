#include <simplicity/workspace.h>
#include <cmath>
#include <limits>
#include "test_support.h"

int main() {
    using simplicity::Workspace;
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    Workspace w;
    CHECK(w.tracks().size() == 4); CHECK(w.title() == "Late hours"); CHECK(!w.canUndo()); CHECK(!w.canRedo());
    CHECK(!w.undo()); CHECK(!w.redo()); CHECK(!w.clip(-1)); CHECK(!w.selectClip(-1));
    CHECK(!w.selectTrack(-1)); CHECK(!w.selectTrack(4));
    CHECK(!w.duplicateClip()); CHECK(!w.removeClip());
    CHECK(!w.setTitle("")); CHECK(!w.setTitle("   ")); CHECK(!w.setTitle(std::string(49, 'x'))); CHECK(!w.setTitle(w.title()));
    CHECK(w.setTitle("Test")); CHECK(w.title() == "Test"); CHECK(w.undo()); CHECK(w.redo());
    CHECK(!w.setBpm(nan)); CHECK(!w.setBpm(inf)); CHECK(!w.setBpm(39)); CHECK(!w.setBpm(241)); CHECK(!w.setBpm(120));
    CHECK(w.setBpm(40)); CHECK(w.setBpm(240)); CHECK(w.setBpm(120));
    CHECK(!w.renameTrack("")); CHECK(!w.renameTrack("Drums")); CHECK(w.renameTrack("Beat"));
    CHECK(!w.setLevel(nan)); CHECK(!w.setLevel(-61)); CHECK(!w.setLevel(7)); CHECK(!w.setLevel(-6));
    CHECK(w.setLevel(-60)); CHECK(w.setLevel(6)); CHECK(w.toggleMute()); CHECK(w.toggleMute()); CHECK(w.toggleSolo()); CHECK(w.toggleSolo());
    CHECK(w.tracks()[0].level == 6); CHECK(!w.tracks()[0].muted); CHECK(!w.tracks()[0].solo);
    CHECK(Workspace::snapped(nan, 1) == 0); CHECK(Workspace::snapped(1.2, 0) == 1.2); CHECK(Workspace::snapped(1.2, nan) == 1.2);
    CHECK(Workspace::snapped(1.2, -1) == 1.2); CHECK(Workspace::snapped(1.2, inf) == 1.2); CHECK(Workspace::snapped(1.2, 0.25) == 1.25);
    const int first = w.tracks()[0].clips[0].id;
    CHECK(w.selectClip(first)); CHECK(w.selectedTrack() == 0); CHECK(w.selectedClip() == first);
    CHECK(!w.moveClip(first, nan)); CHECK(!w.moveClip(-1, 1)); CHECK(!w.moveClip(first, 0));
    CHECK(w.moveClip(first, 3.2)); CHECK(w.clip(first)->beat == 3); CHECK(w.moveClip(first, -99)); CHECK(w.clip(first)->beat == 0);
    CHECK(w.moveClip(first, 1000)); CHECK(w.clip(first)->beat == 48); CHECK(w.duplicateClip());
    CHECK(w.clip(w.selectedClip())->beat == 48); CHECK(w.removeClip()); CHECK(!w.removeClip());
    CHECK(w.selectClip(w.tracks()[3].clips[0].id)); CHECK(w.selectedTrack() == 3);
    CHECK(w.seek(63)); CHECK(w.addClip()); CHECK(w.clip(w.selectedClip())->beat == 60); CHECK(w.clip(w.selectedClip())->length == 4);
    CHECK(!w.seek(nan)); CHECK(w.seek(-1)); CHECK(w.position() == 0); CHECK(w.seek(100)); CHECK(w.position() == 64);
    w.advance(1); CHECK(w.position() == 64); // stopped
    w.togglePlay(); CHECK(w.playing()); CHECK(w.position() == 0);
    w.advance(0); w.advance(-1); w.advance(nan); w.advance(inf); CHECK(w.position() == 0);
    w.advance(0.5); CHECK(w.position() == 1); w.togglePlay(); CHECK(!w.playing());
    w.stop(); CHECK(w.position() == 0);
    CHECK(w.seek(63)); w.togglePlay(); w.advance(1); CHECK(w.position() == 1); CHECK(w.playing());
    w.loop = false; CHECK(w.seek(63)); w.advance(1); CHECK(w.position() == 64); CHECK(!w.playing());
    w.togglePlay(); CHECK(w.position() == 0); w.advance(1000000); CHECK(w.position() == 64); // bounded timer gap
    w.newSession(); CHECK(w.tracks().empty()); CHECK(w.selectedTrack() == -1); CHECK(w.selectedClip() == 0); CHECK(w.title() == "Untitled session");
    CHECK(!w.renameTrack("X")); CHECK(!w.removeTrack()); CHECK(!w.setLevel(0)); CHECK(!w.toggleMute()); CHECK(!w.toggleSolo()); CHECK(!w.addClip());
    CHECK(!w.addTrack("")); CHECK(!w.addTrack(" ")); CHECK(!w.addTrack(std::string(49, 'x')));
    CHECK(w.addTrack("New")); CHECK(w.selectedTrack() == 0); CHECK(w.addClip()); const int newClip = w.selectedClip();
    CHECK(w.selectTrack(0)); CHECK(w.selectedClip() == 0); CHECK(w.selectClip(newClip));
    for (int i = 1; i < 64; ++i) CHECK(w.addClip());
    CHECK(!w.addClip()); CHECK(!w.duplicateClip());
    CHECK(w.removeTrack()); CHECK(w.tracks().empty()); CHECK(w.selectedTrack() == -1);
    CHECK(w.undo()); CHECK(w.tracks().size() == 1); CHECK(w.tracks()[0].clips.size() == 64); CHECK(w.redo());
    for (int i = 0; i < Workspace::maxTracks; ++i) CHECK(w.addTrack("Lane " + std::to_string(i)));
    CHECK(!w.addTrack("Overflow")); CHECK(w.selectTrack(0)); CHECK(w.removeTrack()); CHECK(w.selectedTrack() == 22);
    CHECK(w.undo()); CHECK(w.canRedo()); CHECK(w.setTitle("Branch history")); CHECK(!w.canRedo());
    // Undo history is bounded; repeated edits cannot retain an unbounded session graph.
    for (int i = 0; i < 100; ++i) CHECK(w.setTitle("Edit " + std::to_string(i)));
    int undos = 0; while (w.undo()) ++undos; CHECK(undos == 64);
    int redos = 0; while (w.redo()) ++redos; CHECK(redos == 64);
    w.resetDemo(); CHECK(w.tracks().size() == 4); CHECK(!w.playing()); CHECK(w.position() == 0); CHECK(w.selectedClip() == 0);
    CHECK(w.selectClip(w.tracks()[2].clips[1].id)); CHECK(w.clip(w.selectedClip())->length == 32);
}
