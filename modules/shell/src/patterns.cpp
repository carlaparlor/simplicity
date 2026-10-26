#include <simplicity/workspace.h>
#include <algorithm>
#include <cmath>

namespace simplicity {
namespace {
bool named(const std::string& s) { return !s.empty() && s.size()<=48 && s.find_first_not_of(' ')!=std::string::npos; }
}
const Pattern* Workspace::pattern(int id) const noexcept {
    for (const auto& p:state_.patterns) if(p.id==id) return &p;
    return nullptr;
}
bool Workspace::selectPattern(int id) noexcept {
    if(!pattern(id)) return false;
    state_.activePattern=id; return true;
}
bool Workspace::addPattern(std::string name,int beats) {
    if(!named(name) || beats<1 || beats>64 || state_.patterns.size()>=maxPatterns) return false;
    remember(); state_.patterns.push_back({state_.nextPattern++,beats*ticksPerBeat,static_cast<int>(state_.patterns.size())%4,std::move(name),{}});
    state_.activePattern=state_.patterns.back().id; return true;
}
bool Workspace::renamePattern(std::string name) {
    const auto* p=pattern(state_.activePattern);
    if(!p || !named(name) || p->name==name) return false;
    remember(); for(auto& target:state_.patterns) if(target.id==state_.activePattern) target.name=std::move(name);
    return true;
}
bool Workspace::clonePattern() {
    const auto* p=pattern(state_.activePattern);
    if(!p || state_.patterns.size()>=maxPatterns) return false;
    auto copy=*p; copy.name=copy.name.substr(0,43)+" copy";
    remember(); copy.id=state_.nextPattern++; state_.activePattern=copy.id; state_.patterns.push_back(std::move(copy)); return true;
}
bool Workspace::deletePattern() {
    if(!pattern(state_.activePattern)) return false;
    for(const auto& t:state_.tracks) for(const auto& c:t.clips) if(c.pattern==state_.activePattern) return false;
    remember(); std::erase_if(state_.patterns,[this](const auto& p){return p.id==state_.activePattern;});
    state_.activePattern=state_.patterns.empty()?0:state_.patterns.front().id; return true;
}
bool Workspace::makeUnique() {
    const auto* c=clip(state_.selectedClip);
    if(!c || state_.patterns.size()>=maxPatterns) return false;
    const auto* p=pattern(c->pattern);
    auto copy=*p; copy.name=copy.name.substr(0,43)+" copy";
    remember(); copy.id=state_.nextPattern++; state_.activePattern=copy.id;
    for(auto& t:state_.tracks) for(auto& target:t.clips) if(target.id==state_.selectedClip) target.pattern=copy.id;
    state_.patterns.push_back(std::move(copy)); return true;
}
bool Workspace::addNote(PatternNote note) {
    const auto* p=pattern(state_.activePattern);
    if(!p || note.tick<0 || note.length<1 || note.length>p->length || note.tick>p->length-note.length
       || note.pitch<0 || note.pitch>127 || note.channel<0 || note.channel>3 || note.velocity<1 || note.velocity>127 || p->notes.size()>=maxNotes) return false;
    if(std::any_of(p->notes.begin(),p->notes.end(),[&](const auto& n){return n.tick==note.tick && n.pitch==note.pitch && n.channel==note.channel;})) return false;
    remember(); for(auto& target:state_.patterns) if(target.id==state_.activePattern) target.notes.push_back(note);
    return true;
}
bool Workspace::removeNote(int index) {
    const auto* p=pattern(state_.activePattern);
    if(!p || index<0 || index>=static_cast<int>(p->notes.size())) return false;
    remember(); for(auto& target:state_.patterns) if(target.id==state_.activePattern) target.notes.erase(target.notes.begin()+index);
    return true;
}
bool Workspace::resizeNote(int index,int length){
    const auto* p=pattern(state_.activePattern);
    if(!p || index<0 || index>=static_cast<int>(p->notes.size()) || length<1 || length>p->length-p->notes[static_cast<std::size_t>(index)].tick || length==p->notes[static_cast<std::size_t>(index)].length)return false;
    remember();for(auto& target:state_.patterns)if(target.id==state_.activePattern)target.notes[static_cast<std::size_t>(index)].length=length;return true;
}
bool Workspace::paintPattern(int lane,const std::vector<double>& beats) {
    const auto* p=pattern(state_.activePattern);
    if(!p || lane<0 || lane>=static_cast<int>(state_.tracks.size())) return false;
    const double length=static_cast<double>(p->length)/ticksPerBeat;
    auto copies=state_.tracks[static_cast<std::size_t>(lane)].clips;
    int next=state_.nextId;
    for(const auto beat:beats) {
        if(!std::isfinite(beat) || beat<0 || beat>duration-length) return false;
        const auto at=std::clamp(snapped(beat,grid),0.0,duration-length);
        if(std::any_of(copies.begin(),copies.end(),[&](const auto& c){return c.pattern==p->id && c.beat==at;})) continue;
        if(copies.size()>=64) return false;
        copies.push_back({next++,at,length,p->id,false});
    }
    if(next==state_.nextId) return false;
    remember(); state_.tracks[static_cast<std::size_t>(lane)].clips=std::move(copies);
    state_.nextId=next; state_.selectedTrack=lane; state_.selectedClip=next-1; return true;
}
bool Workspace::placePattern(int lane,double beat) { return paintPattern(lane,{beat}); }
bool Workspace::clonePlacement(int id,int lane,double beat) {
    const auto* c=clip(id);
    if(!c || lane<0 || lane>=static_cast<int>(state_.tracks.size()) || !std::isfinite(beat)) return false;
    auto& target=state_.tracks[static_cast<std::size_t>(lane)].clips;
    if(target.size()>=64) return false;
    auto copy=*c; copy.beat=std::clamp(snapped(beat,grid),0.0,duration-c->length);
    remember(); copy.id=state_.nextId++; state_.selectedClip=copy.id; state_.selectedTrack=lane; target.push_back(copy); return true;
}
bool Workspace::resizeClip(int id,double beats) {
    const auto* c=clip(id);
    if(!c || !std::isfinite(beats) || beats<=0 || beats>duration-c->beat || beats==c->length) return false;
    remember(); for(auto& t:state_.tracks) for(auto& target:t.clips) if(target.id==id) target.length=beats;
    return true;
}
bool Workspace::sliceClip(int id,double beat) {
    const auto* c=clip(id);
    if(!c || !std::isfinite(beat) || beat<=c->beat || beat>=c->beat+c->length) return false;
    for(auto& t:state_.tracks) for(auto& target:t.clips) if(target.id==id) {
        if(t.clips.size()>=64) return false;
        auto right=target; right.offset+=beat-c->beat; right.beat=beat; right.length=c->beat+c->length-beat;
        remember(); target.length=beat-c->beat; right.id=state_.nextId++; t.clips.push_back(right); return true;
    }
    return false;
}
bool Workspace::toggleClipMute(int id) {
    if(!clip(id)) return false;
    remember(); for(auto& t:state_.tracks) for(auto& c:t.clips) if(c.id==id) c.muted=!c.muted;
    return true;
}
bool Workspace::moveClipTo(int id,int lane,double beat,bool useSnap) {
    const auto* c=clip(id);
    if(!c || lane<0 || lane>=static_cast<int>(state_.tracks.size())) return false;
    if(!std::isfinite(beat)) return false;
    const auto at=std::clamp(useSnap?snapped(beat,grid):beat,0.0,duration-c->length);
    auto& destination=state_.tracks[static_cast<std::size_t>(lane)].clips;
    const bool same=std::any_of(destination.begin(),destination.end(),[&](const auto& v){return v.id==id;});
    if(same) {
        if(c->beat==at) return false;
        remember(); for(auto& target:destination) if(target.id==id) target.beat=at;
        return true;
    }
    if(destination.size()>=64) return false;
    auto copy=*c; copy.beat=at;
    remember(); for(auto& t:state_.tracks) std::erase_if(t.clips,[&](const auto& v){return v.id==id;});
    destination.push_back(copy); state_.selectedTrack=lane; state_.selectedClip=id; return true;
}
double Workspace::transportLength() const noexcept {
    const auto* p=pattern(state_.activePattern);
    return patternMode && p ? static_cast<double>(p->length)/ticksPerBeat : duration;
}
void Workspace::seedPatterns() {
    const char* names[]{"Drums A","Bass A","Chords A","Atmosphere"};
    for(int i=0;i<4;++i) state_.patterns.push_back({state_.nextPattern++,384,i,names[i],{}});
    auto& drums=state_.patterns[0].notes;
    for(int step=0;step<16;++step) {
        if(step%4==0) drums.push_back({step*24,12,36,0,110});
        if(step%8==4) drums.push_back({step*24,12,38,1,105});
        if(step%2==0) drums.push_back({step*24,8,42,2,75});
    }
    for(int i=0;i<4;++i) state_.patterns[1].notes.push_back({i*96,72,36+(i%2)*7,3,100});
    for(const int pitch:{60,64,67}) state_.patterns[2].notes.push_back({0,360,pitch,3,90});
    for(const int pitch:{72,79}) state_.patterns[3].notes.push_back({0,384,pitch,3,70});
    state_.activePattern=state_.patterns.front().id;
}
}
