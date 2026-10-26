#include <simplicity/workspace.h>
#include <limits>
#include "test_support.h"

int main(){
    using simplicity::Workspace;using simplicity::PatternNote;
    const auto nan=std::numeric_limits<double>::quiet_NaN();
    Workspace w;CHECK(w.patterns().size()==4);CHECK(w.pattern(1)->notes.size()==14);CHECK(w.activePattern()==1);
    CHECK(!w.pattern(999));CHECK(!w.selectPattern(999));CHECK(w.selectPattern(2));CHECK(w.activePattern()==2);
    CHECK(!w.addPattern(""));CHECK(!w.addPattern("  "));CHECK(!w.addPattern(std::string(49,'a')));CHECK(!w.addPattern("Bad",0));CHECK(!w.addPattern("Bad",65));
    CHECK(w.addPattern("Melody",8));const int melody=w.activePattern();CHECK(w.pattern(melody)->length==768);
    CHECK(!w.renamePattern(""));CHECK(!w.renamePattern("Melody"));CHECK(w.renamePattern("Lead"));CHECK(w.pattern(melody)->name=="Lead");
    CHECK(!w.resizeNote(-1,24));CHECK(!w.resizeNote(0,24));
    CHECK(w.addNote({0,96,60,3,100}));CHECK(!w.resizeNote(0,0));CHECK(!w.resizeNote(0,769));CHECK(!w.resizeNote(0,96));CHECK(w.resizeNote(0,192));CHECK(w.pattern(melody)->notes[0].length==192);CHECK(w.undo());CHECK(w.addNote({96,48,64,3,90}));CHECK(!w.addNote({0,24,60,3,110}));
    CHECK(w.pattern(melody)->notes.size()==2);CHECK(w.placePattern(0,4));const int a=w.selectedClip();CHECK(w.placePattern(1,12));const int b=w.selectedClip();
    CHECK(w.clip(a)->pattern==melody);CHECK(w.clip(b)->pattern==melody);CHECK(w.addNote({192,192,67,3,80}));CHECK(w.pattern(w.clip(a)->pattern)->notes.size()==3);CHECK(w.pattern(w.clip(b)->pattern)->notes.size()==3);
    CHECK(!w.deletePattern());CHECK(w.selectClip(b));CHECK(w.makeUnique());const int unique=w.activePattern();CHECK(unique!=melody);CHECK(w.clip(b)->pattern==unique);CHECK(w.clip(a)->pattern==melody);
    CHECK(w.removeNote(0));CHECK(w.pattern(unique)->notes.size()==2);CHECK(w.pattern(melody)->notes.size()==3);CHECK(w.undo());CHECK(w.pattern(unique)->notes.size()==3);CHECK(w.undo());CHECK(w.clip(b)->pattern==melody);CHECK(!w.pattern(unique));CHECK(w.redo());CHECK(w.clip(b)->pattern==unique);
    CHECK(!w.removeNote(-1));CHECK(!w.removeNote(100));CHECK(w.clonePattern());const int copy=w.activePattern();CHECK(w.pattern(copy)->notes.size()==3);CHECK(w.deletePattern());CHECK(!w.pattern(copy));
    CHECK(w.selectPattern(melody));CHECK(w.selectClip(a));CHECK(w.resizeClip(a,12));CHECK(w.clip(a)->length==12);CHECK(!w.resizeClip(a,12));CHECK(!w.resizeClip(999,4));CHECK(!w.resizeClip(a,nan));CHECK(!w.resizeClip(a,0));CHECK(!w.resizeClip(a,61));
    CHECK(!w.sliceClip(999,6));CHECK(!w.sliceClip(a,nan));CHECK(!w.sliceClip(a,4));CHECK(!w.sliceClip(a,16));CHECK(w.sliceClip(a,7));CHECK(w.clip(a)->length==3);
    const auto& pieces=w.tracks()[0].clips;bool offsetFound=false;for(const auto& c:pieces)if(c.beat==7 && c.pattern==melody){CHECK(c.offset==3);CHECK(c.length==9);offsetFound=true;}CHECK(offsetFound);CHECK(w.undo());CHECK(w.clip(a)->length==12);
    CHECK(!w.toggleClipMute(999));CHECK(w.toggleClipMute(a));CHECK(w.clip(a)->muted);CHECK(w.toggleClipMute(a));CHECK(!w.clip(a)->muted);
    CHECK(!w.moveClipTo(999,0,2));CHECK(!w.moveClipTo(a,-1,2));CHECK(!w.moveClipTo(a,99,2));CHECK(!w.moveClipTo(a,0,nan));CHECK(!w.moveClipTo(a,0,4));
    w.grid=1;CHECK(w.moveClipTo(a,0,4.25,false));CHECK(w.clip(a)->beat==4.25);CHECK(w.moveClipTo(a,0,5.3));CHECK(w.clip(a)->beat==5);CHECK(w.moveClipTo(a,2,9));CHECK(w.selectedTrack()==2);CHECK(w.clip(a)->beat==9);
    CHECK(!w.clonePlacement(999,0,0));CHECK(!w.clonePlacement(a,-1,0));CHECK(!w.clonePlacement(a,99,0));CHECK(!w.clonePlacement(a,0,nan));CHECK(w.clonePlacement(a,0,0));CHECK(w.clip(w.selectedClip())->pattern==melody);
    CHECK(!w.paintPattern(-1,{0}));CHECK(!w.paintPattern(99,{0}));CHECK(!w.paintPattern(0,{nan}));CHECK(!w.paintPattern(0,{-1}));CHECK(!w.paintPattern(0,{64}));CHECK(!w.paintPattern(0,{}));
    CHECK(w.selectPattern(4));CHECK(w.addTrack("Paint lane"));const int lane=w.selectedTrack();CHECK(w.paintPattern(lane,{0,4,8,8}));CHECK(w.tracks().back().clips.size()==3);CHECK(!w.paintPattern(lane,{0,4,8}));CHECK(w.undo());CHECK(w.tracks().back().clips.empty());CHECK(w.redo());CHECK(w.tracks().back().clips.size()==3);
    CHECK(w.selectTrack(lane));CHECK(!w.setPan(nan));CHECK(!w.setPan(-2));CHECK(!w.setPan(2));CHECK(!w.setPan(0));CHECK(w.setPan(-1));CHECK(w.setPan(1));CHECK(w.tracks().back().pan==1);CHECK(w.undo());CHECK(w.tracks().back().pan==-1);
    w.newSession();CHECK(w.patterns().empty());CHECK(!w.renamePattern("No"));CHECK(!w.clonePattern());CHECK(!w.deletePattern());CHECK(!w.makeUnique());CHECK(!w.addNote({}));CHECK(!w.resizeNote(0,24));CHECK(!w.removeNote(0));CHECK(!w.paintPattern(0,{0}));CHECK(!w.setPan(0));
    w.patternMode=true;w.togglePlay();w.advance(0.5);CHECK(w.position()==1);w.stop();CHECK(w.addPattern("One",1));CHECK(w.addTrack("Lane"));CHECK(w.placePattern(0,0));CHECK(w.selectClip(w.selectedClip()));CHECK(!w.deletePattern());
    w.loop=true;w.togglePlay();w.advance(0.75);CHECK(w.position()==0.5);CHECK(w.playing());w.loop=false;w.advance(1);CHECK(w.position()==1);CHECK(!w.playing());w.togglePlay();CHECK(w.position()==0);w.stop();
    CHECK(w.addPattern("Notes",64));
    for(auto note: {PatternNote{-1,1,60,3,100},PatternNote{0,0,60,3,100},PatternNote{0,6145,60,3,100},PatternNote{6144,1,60,3,100},PatternNote{0,1,-1,3,100},PatternNote{0,1,128,3,100},PatternNote{0,1,60,-1,100},PatternNote{0,1,60,4,100},PatternNote{0,1,60,3,0},PatternNote{0,1,60,3,128}})CHECK(!w.addNote(note));
    CHECK(w.addNote({0,6144,0,0,1}));CHECK(w.addNote({0,6144,127,3,127}));CHECK(w.removeNote(0));CHECK(w.removeNote(0));
    for(int n=0;n<Workspace::maxNotes;++n){CHECK(w.addNote({n,1,60,3,100}));}CHECK(!w.addNote({513,1,60,3,100}));
    CHECK(w.selectPattern(1));CHECK(w.selectTrack(0));for(int n=1;n<64;++n)CHECK(w.addClip());CHECK(!w.clonePlacement(w.tracks()[0].clips[0].id,0,1));CHECK(!w.sliceClip(w.tracks()[0].clips[1].id,0.5));CHECK(!w.moveClipTo(w.tracks()[0].clips[0].id,0,0));CHECK(!w.paintPattern(0,{63}));
    CHECK(w.addTrack("Other"));CHECK(w.placePattern(1,0));const int other=w.selectedClip();CHECK(!w.moveClipTo(other,0,1));CHECK(w.selectClip(other));
    while(w.patterns().size()<Workspace::maxPatterns){CHECK(w.addPattern("Source"));}CHECK(!w.addPattern("Overflow"));CHECK(!w.clonePattern());CHECK(!w.makeUnique());
    CHECK(w.selectPattern(1));CHECK(w.removeClip());CHECK(w.selectTrack(0));CHECK(w.removeTrack());CHECK(w.deletePattern());CHECK(w.pattern(1)==nullptr);
    w.newSession();CHECK(w.addPattern("Only"));CHECK(w.deletePattern());CHECK(w.activePattern()==0);CHECK(!w.canRedo());
}
