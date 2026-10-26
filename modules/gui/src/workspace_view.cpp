#include <simplicity/workspace_view.h>
#include <simplicity/ui_style.h>
#include <simplicity/browser_panel.h>
#include <simplicity/pattern_panel.h>
#include <algorithm>
#include <cmath>
#include <functional>

namespace simplicity {
namespace {
const auto bg = ui::background, panel = ui::panel, line = ui::border, ink = ui::foreground, muted = ui::secondary;
const juce::Colour trackColours[]{juce::Colour{0xffa4c98e}, juce::Colour{0xffcba57e}, juce::Colour{0xff8daabf}, juce::Colour{0xffb1a0c9}};
constexpr int rowHeight = 64;
juce::String number(int n) { return juce::String(n).paddedLeft('0', 2); }
class Canvas final : public juce::Component, public juce::DragAndDropTarget {
public:
    enum class Tool { draw, paint, select, erase, mute, slice, zoom };
    Workspace& model;
    std::function<void()> changed, edit, zoomRequested;
    double beatWidth{12};
    Tool tool{Tool::draw};
    int dragged{}, dragRow{-1}, targetRow{-1};
    double dragStart{}, preview{}, previewLength{};
    bool cloning{}, trimming{}, freeSnap{}, painting{};
    std::vector<double> stroke;
    explicit Canvas(Workspace& m):model(m){setName("Playlist. P draw, B paint, E select. Shift drag clones, Alt bypasses snap.");setWantsKeyboardFocus(true);setComponentID("playlist");}
    juce::Rectangle<float> clipBounds(const SketchClip& c,int row) const {
        const bool active=dragged==c.id;
        const double at=active?preview:c.beat;
        return {static_cast<float>(at*beatWidth+1),static_cast<float>((active?targetRow:row)*rowHeight+5),static_cast<float>((active?previewLength:c.length)*beatWidth-2),static_cast<float>(rowHeight-10)};
    }
    void paint(juce::Graphics& g) override {
        g.fillAll(bg);
        for(int bar=0;bar<16;++bar){const int x=static_cast<int>(bar*4*beatWidth);if((bar/4)%2==0){g.setColour(juce::Colour(0xff181818));g.fillRect(x,0,static_cast<int>(4*beatWidth),getHeight());}g.setColour(line);g.drawVerticalLine(x,0,static_cast<float>(getHeight()));for(int b=1;b<4;++b){g.setColour(line.withAlpha(0.35F));g.drawVerticalLine(x+static_cast<int>(b*beatWidth),0,static_cast<float>(getHeight()));}}
        for(std::size_t row=0;row<model.tracks().size();++row){const auto& track=model.tracks()[row];g.setColour(line);g.drawHorizontalLine(static_cast<int>(row+1)*rowHeight-1,0,static_cast<float>(getWidth()));
            for(const auto& c:track.clips){const auto r=clipBounds(c,static_cast<int>(row));if(!g.getClipBounds().intersects(r.toNearestInt()))continue;const auto* p=model.pattern(c.pattern);if(!p)continue;const auto colour=trackColours[p->colour%4];const bool quiet=track.muted||c.muted;
                g.setColour(juce::Colours::black.withAlpha(0.5F));g.fillRoundedRectangle(r.translated(1,2),2);
                g.setColour(colour.interpolatedWith(bg,quiet?0.93F:0.78F));g.fillRoundedRectangle(r,2);
                g.setGradientFill(juce::ColourGradient(colour.withAlpha(quiet?0.12F:0.5F),r.getX(),r.getY(),colour.withAlpha(quiet?0.08F:0.28F),r.getX(),r.getY()+18,false));g.fillRect(r.withHeight(18));
                ui::label(g,juce::String(p->name),r.toNearestInt().reduced(5,0).withHeight(18),12,quiet?muted:ink);
                drawPatternNotes(g,*p,r.reduced(3,3).withTrimmedTop(18),c.offset,dragged==c.id?previewLength:c.length,colour.withAlpha(quiet?0.25F:0.95F));
                g.setColour(model.selectedClip()==c.id?ink:colour.withAlpha(0.3F));g.drawRoundedRectangle(r,2,1);
            }
        }
        if(painting){g.setColour(ink.withAlpha(0.22F));const auto* p=model.pattern(model.activePattern());if(p)for(auto at:stroke)g.fillRect(static_cast<float>(at*beatWidth),static_cast<float>(dragRow*rowHeight+5),static_cast<float>(p->length)/96*static_cast<float>(beatWidth),static_cast<float>(rowHeight-10));}
        g.setColour(ink);g.fillRect(static_cast<float>(model.position()*beatWidth),0.0F,1.0F,static_cast<float>(getHeight()));
    }
    const SketchClip* hit(juce::Point<float> at) const {for(std::size_t row=0;row<model.tracks().size();++row)for(const auto& c:model.tracks()[row].clips)if(clipBounds(c,static_cast<int>(row)).contains(at))return &c;return nullptr;}
    void mouseDown(const juce::MouseEvent& e) override {
        grabKeyboardFocus();dragged=0;painting=false;stroke.clear();dragRow=targetRow=e.y/rowHeight;freeSnap=e.mods.isAltDown();const double at=e.position.x/beatWidth;
        if(tool==Tool::zoom){if(zoomRequested)zoomRequested();return;}
        if(const auto* found=hit(e.position)){const auto c=*found;model.selectClip(c.id);model.selectPattern(c.pattern);
            if(e.mods.isRightButtonDown() || tool==Tool::erase){model.removeClip();changed();return;}
            if(tool==Tool::mute){model.toggleClipMute(c.id);changed();return;}
            if(tool==Tool::slice){model.sliceClip(c.id,Workspace::snapped(at,model.grid));changed();return;}
            dragged=c.id;dragStart=c.beat;preview=c.beat;previewLength=c.length;cloning=e.mods.isShiftDown();trimming=!cloning && e.position.x>static_cast<float>((c.beat+c.length)*beatWidth-6);changed();return;
        }
        if(dragRow<0 || dragRow>=static_cast<int>(model.tracks().size()))return;
        model.selectTrack(dragRow);
        if(tool==Tool::draw && !e.mods.isRightButtonDown())model.placePattern(dragRow,at);
        else if(tool==Tool::paint && !e.mods.isRightButtonDown()){painting=true;stroke.push_back(Workspace::snapped(at,model.grid));}
        else model.seek(at);
        changed();
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        if(painting){const auto* p=model.pattern(model.activePattern());if(!p)return;const double length=static_cast<double>(p->length)/96;const double end=e.position.x/beatWidth;const double start=stroke.front();stroke.resize(1);for(double at=start+length;at<=end && at<=Workspace::duration-length;at+=length)stroke.push_back(at);repaint();return;}
        if(const auto* c=model.clip(dragged)){const double delta=e.getDistanceFromDragStartX()/beatWidth;const auto value=freeSnap?dragStart+delta:Workspace::snapped(dragStart+delta,model.grid);if(trimming)previewLength=std::clamp(freeSnap?c->length+delta:Workspace::snapped(c->length+delta,model.grid),0.25,Workspace::duration-c->beat);else{preview=std::clamp(value,0.0,Workspace::duration-c->length);targetRow=std::clamp(e.y/rowHeight,0,static_cast<int>(model.tracks().size())-1);}repaint();}
    }
    void mouseUp(const juce::MouseEvent&) override {
        if(painting){painting=false;model.paintPattern(dragRow,stroke);stroke.clear();changed();}
        if(dragged!=0){const int id=dragged;dragged=0;if(trimming)model.resizeClip(id,previewLength);else if(cloning)model.clonePlacement(id,targetRow,preview);else model.moveClipTo(id,targetRow,preview,!freeSnap);changed();}
    }
    bool isInterestedInDragSource(const SourceDetails& details) override {const auto text=details.description.toString();return text.startsWith("pattern:") && model.pattern(text.fromFirstOccurrenceOf(":",false,false).getIntValue())!=nullptr;}
    void itemDropped(const SourceDetails& details) override {if(!isInterestedInDragSource(details))return;model.selectPattern(details.description.toString().fromFirstOccurrenceOf(":",false,false).getIntValue());model.placePattern(details.localPosition.y/rowHeight,details.localPosition.x/beatWidth);changed();}
    void mouseDoubleClick(const juce::MouseEvent& e) override {if(const auto* c=hit(e.position)){model.selectPattern(c->pattern);if(edit)edit();}}
};
class ResizeGrip final : public juce::Component {
public:
    std::function<void(int)> resize;
    int last{};
    bool vertical{};
    explicit ResizeGrip(bool v=false):vertical(v){setMouseCursor(v?juce::MouseCursor::UpDownResizeCursor:juce::MouseCursor::LeftRightResizeCursor);setName("Resize panel");}
    void mouseDown(const juce::MouseEvent&) override {last=0;}
    void mouseDrag(const juce::MouseEvent& e) override {const int value=vertical?e.getDistanceFromDragStartY():e.getDistanceFromDragStartX();if(resize)resize(value-last);last=value;}
};
class ScrollingView final : public juce::Viewport {
public:
    std::function<void()> scrolled;
    void visibleAreaChanged(const juce::Rectangle<int>&) override { if (scrolled) scrolled(); }
};
class TrackName final : public juce::Label {
public:
    std::function<void()> selected;
    bool active{};
    TrackName() { setEditable(false, true); setFont(ui::font(14)); setBorderSize({0, 8, 0, 6}); }
    void mouseDown(const juce::MouseEvent& e) override { if (selected) selected(); juce::Label::mouseDown(e); }
    void paint(juce::Graphics& g) override { if (active) { g.setColour(ui::selected); g.fillRoundedRectangle(getLocalBounds().toFloat(), ui::cornerRadius); } juce::Label::paint(g); }
};
class TrackHeaders final : public juce::Component {
public:
    Workspace& model; std::function<void()> changed; int scroll{};
    juce::OwnedArray<TrackName> names;
    juce::OwnedArray<juce::TextButton> mute, solo;
    explicit TrackHeaders(Workspace& m) : model(m) {}
    void refresh() {
        if (names.size() != static_cast<int>(model.tracks().size())) {
            names.clear(); mute.clear(); solo.clear();
            for (int i = 0; i < static_cast<int>(model.tracks().size()); ++i) {
                auto* n = names.add(new TrackName()); auto* m = mute.add(new juce::TextButton("M")); auto* s = solo.add(new juce::TextButton("S"));
                addAndMakeVisible(n); addAndMakeVisible(m); addAndMakeVisible(s);
                n->selected = [this, i] { model.selectTrack(i); changed(); };
                n->onTextChange = [this, i] { model.selectTrack(i); model.renameTrack(names[i]->getText().toStdString()); changed(); };
                m->onClick = [this, i] { model.selectTrack(i); model.toggleMute(); changed(); };
                s->onClick = [this, i] { model.selectTrack(i); model.toggleSolo(); changed(); };
                n->setTooltip("Select track; double-click to rename"); m->setTooltip("Mute (sketch state)"); s->setTooltip("Solo (sketch state)");
            }
        }
        for (int i = 0; i < names.size(); ++i) {
            const auto& t = model.tracks()[static_cast<std::size_t>(i)];
            if (!names[i]->isBeingEdited()) names[i]->setText(t.name, juce::dontSendNotification);
            names[i]->active = model.selectedTrack() == i; names[i]->setName("Track " + juce::String(t.name)); names[i]->repaint();
            mute[i]->setToggleState(t.muted, juce::dontSendNotification); solo[i]->setToggleState(t.solo, juce::dontSendNotification);
            mute[i]->setTitle("Mute " + juce::String(t.name)); solo[i]->setTitle("Solo " + juce::String(t.name));
        }
        resized(); repaint();
    }
    void resized() override { for (int i = 0; i < names.size(); ++i) { const int y = i*rowHeight-scroll; names[i]->setBounds(30,y+10,getWidth()-42,27); mute[i]->setBounds(30,y+37,24,20); solo[i]->setBounds(60,y+37,24,20); } }
    void paint(juce::Graphics& g) override {
        g.fillAll(panel);
        for (int i = 0; i < names.size(); ++i) {
            const int y = i*rowHeight-scroll; const auto colour = trackColours[model.tracks()[static_cast<std::size_t>(i)].colour % 4];
            ui::glyph(g, ui::Glyph::wave, {8, static_cast<float>(y+15), 16, 16}, colour);
            ui::label(g, number(i+1), {8,y+37,20,20},14, muted);
            g.setColour(line.withAlpha(0.5F)); g.drawHorizontalLine(y+rowHeight-1, 0, static_cast<float>(getWidth()));
        }
    }
};
class Channel final : public juce::Component {
public:
    Workspace& model; int index; std::function<void()> changed;
    juce::TextButton select, mute{"M"}, solo{"S"}; juce::Slider level, pan;
    Channel(Workspace& m, int i) : model(m), index(i) {
        for (auto* c : std::initializer_list<juce::Component*>{&select,&mute,&solo,&level,&pan}) addAndMakeVisible(c);
        level.setSliderStyle(juce::Slider::LinearVertical); level.setTextBoxStyle(juce::Slider::TextBoxBelow,false,68,21); level.setRange(-60,6,0.1); level.setTextValueSuffix(" dB"); level.setDoubleClickReturnValue(true,0);
        level.setName("Track level, dB"); level.setTooltip("Track level (visual sketch only)");
        level.onDragEnd = [this] { model.selectTrack(index); model.setLevel(level.getValue()); changed(); };
        level.onValueChange = [this] { if (!level.isMouseButtonDown()) { model.selectTrack(index); model.setLevel(level.getValue()); if (changed) changed(); } };
        pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);pan.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);pan.setRange(-1,1,0.01);pan.setDoubleClickReturnValue(true,0);pan.setName("Channel pan");pan.setTooltip("Pan, MIDI workspace state only; no audio engine");
        pan.onValueChange=[this]{model.selectTrack(index);model.setPan(pan.getValue());if(changed)changed();};
        select.onClick = [this] { model.selectTrack(index); changed(); };
        mute.onClick = [this] { model.selectTrack(index); model.toggleMute(); changed(); };
        solo.onClick = [this] { model.selectTrack(index); model.toggleSolo(); changed(); };
        mute.setTooltip("Mute (sketch state)"); solo.setTooltip("Solo (sketch state)");
    }
    void refresh() { const auto& t = model.tracks()[static_cast<std::size_t>(index)]; select.setButtonText(t.name); select.setToggleState(model.selectedTrack()==index,juce::dontSendNotification); mute.setToggleState(t.muted,juce::dontSendNotification); solo.setToggleState(t.solo,juce::dontSendNotification); if (!level.isMouseButtonDown()) level.setValue(t.level,juce::dontSendNotification); if(!pan.isMouseButtonDown())pan.setValue(t.pan,juce::dontSendNotification);repaint(); }
    void resized() override { select.setBounds(4,7,getWidth()-8,27); mute.setBounds(8,43,26,22); solo.setBounds(42,43,26,22); pan.setBounds(23,68,32,32);level.setBounds(3,106,64,getHeight()-111); }
    void paint(juce::Graphics& g) override {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff292929),0,0,panel,static_cast<float>(getWidth()),0,false));g.fillAll();
        g.setColour(line);g.drawVerticalLine(getWidth()-1,0,static_cast<float>(getHeight()));
        for(int tick=0;tick<6;++tick){const int y=112+tick*std::max(1,getHeight()-150)/5;g.setColour(ui::secondary.withAlpha(0.4F));g.drawHorizontalLine(y,6,15);}

        g.setColour(trackColours[model.tracks()[static_cast<std::size_t>(index)].colour % 4]); g.fillRoundedRectangle(9,0,static_cast<float>(getWidth()-18),2,1);
        g.setColour(bg); g.fillRoundedRectangle(static_cast<float>(getWidth()-9),109,4,static_cast<float>(std::max(1,getHeight()-149)),2);
    }
};
class HelpPanel final : public juce::Component {
public:
    juce::TextButton close{"Close"};
    HelpPanel() { addAndMakeVisible(close); setAlwaysOnTop(true); }
    void resized() override { close.setBounds(getWidth()/2+115,getHeight()/2+150,105,32); }
    void paint(juce::Graphics& g) override {
        g.fillAll(bg.withAlpha(0.95F)); auto r = juce::Rectangle<int>(getWidth()/2-250,getHeight()/2-210,500,410);
        g.setColour(panel); g.fillRoundedRectangle(r.toFloat(),6); r = r.reduced(30);
        ui::label(g,"Shortcuts",r.removeFromTop(42),28,ink,true);
        for (const auto& s : {"Space stop / play   Ctrl Space pause", "F6 Rack   F7 Piano   F9 Mixer   L PAT/SONG", "Alt F8 Browser   Alt P Patterns   Ctrl F Search", "P Draw   B Paint   C Slice   E Select   T Mute", "F4 New pattern   F2 Rename   Ctrl B Duplicate", "Shift drag clone   Alt bypass snap   Ctrl Z/Y undo"}) ui::label(g,s,r.removeFromTop(31),14);
        ui::label(g,"In-memory note editing. Audio engine not connected.",r.removeFromTop(40),14,muted);
        ui::label(g,"No audio, recording, import or project saving yet.",r.removeFromTop(28),14,muted);
    }
};
} // namespace

class WorkspaceView::Impl final : private juce::Timer {
public:
    WorkspaceView& owner; Workspace model; ui::Look look;
    juce::TooltipWindow tooltips{&owner,500};
    juce::Label title, position;
    juce::TextEditor tempo;
    ui::IconButton file{"Project menu",ui::Glyph::folder}, play{"Play",ui::Glyph::play}, stop{"Stop",ui::Glyph::stop}, record{"Record",ui::Glyph::record}, undo{"Undo",ui::Glyph::undo}, redo{"Redo",ui::Glyph::redo};
    ui::IconButton browserToggle{"Browser",ui::Glyph::tracks}, mixerToggle{"Mixer",ui::Glyph::mixer}, loop{"Loop",ui::Glyph::loop}, help{"Help",ui::Glyph::help};
    ui::IconButton addTrack{"Track",ui::Glyph::plus,true}, addClip{"Pattern",ui::Glyph::pattern,true}, duplicate{"Duplicate clip",ui::Glyph::duplicate}, remove{"Delete selection",ui::Glyph::trash}, zoomOut{"Zoom out",ui::Glyph::minus}, zoomIn{"Zoom in",ui::Glyph::plus};
    ui::IconButton preview{"Preview",ui::Glyph::help,true};
    juce::ComboBox snap;
    BrowserPanel browser;
    PatternPanel patterns{model}; PatternEditor editor{model};
    juce::TextButton mode{"SONG"}, rack{"Rack"}, piano{"Piano"}, picker{"Patterns"};
    juce::ComboBox tool;
    int dock{}, browserWidth{280}, patternWidth{228}, dockHeight{290}; bool showPatterns{true}, playlistMax{};
    ResizeGrip browserGrip, patternGrip, dockGrip{true};
    Canvas canvas{model}; ScrollingView lanes; TrackHeaders headers{model};
    juce::Component mixerContent; juce::Viewport mixerView;
    juce::OwnedArray<Channel> channels;
    HelpPanel helpPanel;
    bool showBrowser{true}, showMixer{true}, helpOpen{};
    double zoom{1}, previousTime{juce::Time::getMillisecondCounterHiRes()};
    juce::Rectangle<int> toolbarArea, browserArea, arrangementArea, rulerArea, mixerArea, statusArea;
    explicit Impl(WorkspaceView& o) : owner(o) {
        owner.setLookAndFeel(&look); owner.setWantsKeyboardFocus(true); owner.setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer); owner.setName("Simplicity workspace");
        for (auto* c : std::initializer_list<juce::Component*>{&title,&position,&tempo,&file,&play,&stop,&record,&undo,&redo,&browserToggle,&mixerToggle,&loop,&help,&addTrack,&addClip,&duplicate,&remove,&zoomOut,&zoomIn,&preview,&snap,&browser,&patterns,&editor,&browserGrip,&patternGrip,&dockGrip,&mode,&rack,&piano,&picker,&tool,&lanes,&headers,&mixerView}) owner.addAndMakeVisible(c);
        owner.addChildComponent(helpPanel);
        title.setFont(ui::font(14,true)); title.setEditable(false,true); title.setTooltip("Double-click to rename project"); title.onTextChange = [this] { model.setTitle(title.getText().toStdString()); refresh(); };
        position.setFont(ui::font(28)); position.setBorderSize({0,0,0,0}); position.setTooltip("Visual position: bar, beat, fraction");
        tempo.setName("Tempo, 40 to 240 BPM"); tempo.setFont(ui::font(14)); tempo.setInputRestrictions(3,"0123456789"); tempo.setBorder({5,8,4,4});
        tempo.onReturnKey = [this] { model.setBpm(tempo.getText().getDoubleValue()); refresh(); owner.grabKeyboardFocus(); };
        tempo.onFocusLost = [this] { model.setBpm(tempo.getText().getDoubleValue()); refresh(); };
        bind(play,ShellAction::playPause,"Visual play / pause (Ctrl Space); Space starts / stops"); play.getProperties().set("primary",true);
        bind(stop,ShellAction::stop,"Stop and return to start (Home)"); record.setEnabled(false); record.setTooltip("Recording is not available yet");
        bind(undo,ShellAction::undo,"Undo (Ctrl Z)"); bind(redo,ShellAction::redo,"Redo (Ctrl Y)");
        bind(browserToggle,ShellAction::browser,"Browser (Alt F8)"); bind(mixerToggle,ShellAction::mixer,"Mixer (F9)");
        bind(addTrack,ShellAction::addTrack,"Add an empty track"); bind(addClip,ShellAction::addClip,"Place active pattern at playhead");
        bind(duplicate,ShellAction::duplicate,"Duplicate selected clip (Ctrl B)"); bind(remove,ShellAction::remove,"Remove selected clip or lane; Delete key removes only clips");
        bind(help,ShellAction::help,"Help and shortcuts"); bind(preview,ShellAction::help,"Visual preview only: no audio, recording or saving. Open help for details.");
        preview.getProperties().set("flat",true);
        loop.onClick = [this] { model.loop = !model.loop; refresh(); }; loop.setTooltip("Loop the visual 16-bar sketch");
        file.onClick = [this] {
            juce::PopupMenu menu; menu.addItem(1,"New project",true); menu.addItem(2,"Demo project",true); menu.addSeparator(); menu.addItem(3,"Save (not available yet)",false);
            auto safe = juce::Component::SafePointer<WorkspaceView>(&owner);
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&file), [safe](int item) { if (safe != nullptr && item != 0) safe->perform(item==1 ? ShellAction::newSession : ShellAction::demo); });
        };
        snap.addItem("Bar",1); snap.addItem("1/2 beat",2); snap.addItem("1/4 beat",3); snap.addItem("Off",4); snap.setSelectedId(3,juce::dontSendNotification); model.grid = 0.25;
        snap.setName("Clip snap grid"); snap.setTooltip("Clip movement snap grid"); snap.onChange = [this] { const double values[]{4,0.5,0.25,0}; model.grid = values[std::clamp(snap.getSelectedId()-1,0,3)]; };
        zoomOut.onClick = [this] { zoom = std::max(0.75,zoom-0.25); owner.resized(); };
        zoomIn.onClick = [this] { zoom = std::min(3.0,zoom+0.25); owner.resized(); };
        lanes.setViewedComponent(&canvas,false); lanes.setScrollBarsShown(true,true); lanes.setScrollBarThickness(7);
        lanes.scrolled = [this] { headers.scroll=lanes.getViewPositionY(); headers.resized(); headers.repaint(); owner.repaint(rulerArea); };
        canvas.zoomRequested=[this]{zoomIn.onClick();};canvas.changed = [this] { refresh(); }; headers.changed = [this] { refresh(); };
        patterns.changed=[this]{refresh();}; editor.changed=[this]{refresh();};
        patterns.edit=[this]{dock=2;showMixer=true;refresh();editor.grabKeyboardFocus();};canvas.edit=patterns.edit;
        rack.onClick=[this]{dock=1;showMixer=true;refresh();editor.grabKeyboardFocus();};piano.onClick=patterns.edit;
        picker.onClick=[this]{showPatterns=!showPatterns;refresh();};
        browserGrip.resize=[this](int delta){browserWidth=std::clamp(browserWidth+delta,240,360);owner.resized();};
        patternGrip.resize=[this](int delta){patternWidth=std::clamp(patternWidth-delta,190,300);owner.resized();};
        dockGrip.resize=[this](int delta){dockHeight=std::clamp(dockHeight-delta,210,350);owner.resized();};
        mode.onClick=[this]{model.patternMode=!model.patternMode;model.stop();refresh();};
        const char* tools[]{"Draw (P)","Paint (B)","Select (E)","Delete (D)","Mute (T)","Slice (C)","Zoom (Z)"};for(int n=0;n<7;++n)tool.addItem(tools[n],n+1);tool.setSelectedId(1,juce::dontSendNotification);tool.setName("Playlist editing tool");tool.onChange=[this]{canvas.tool=static_cast<Canvas::Tool>(tool.getSelectedId()-1);};
        mixerView.setViewedComponent(&mixerContent,false); mixerView.setScrollBarsShown(false,true); mixerView.setScrollBarThickness(7);
        helpPanel.close.onClick = [this] { owner.perform(ShellAction::help); };
        owner.sendLookAndFeelChange(); setIDs(); startTimerHz(30);
    }
    ~Impl() override { stopTimer(); owner.setLookAndFeel(nullptr); }
    void setIDs() {
        file.setComponentID("session-menu"); play.setComponentID("play"); stop.setComponentID("stop"); addTrack.setComponentID("add-track"); addClip.setComponentID("add-clip"); tempo.setComponentID("tempo"); snap.setComponentID("snap");
        duplicate.setComponentID("duplicate"); remove.setComponentID("remove"); browserToggle.setComponentID("browser"); mixerToggle.setComponentID("mixer");
    }
    void bind(ui::IconButton& b, ShellAction action, const juce::String& tooltip) { b.setTooltip(tooltip); b.onClick = [this,action] { owner.perform(action); }; }
    void wireShortcuts(juce::Component& c) { c.addKeyListener(&owner); for (int n=0;n<c.getNumChildComponents();++n) wireShortcuts(*c.getChildComponent(n)); }
    void refresh() {
        title.setText(model.title(),juce::dontSendNotification);
        if (!tempo.hasKeyboardFocus(true)) tempo.setText(juce::String(model.bpm(),0),false);
        addClip.setEnabled(model.selectedTrack()>=0); duplicate.setEnabled(model.selectedClip()!=0); remove.setEnabled(model.selectedTrack()>=0);
        addTrack.setEnabled(model.tracks().size()<Workspace::maxTracks); undo.setEnabled(model.canUndo()); redo.setEnabled(model.canRedo());
        play.symbol = model.playing() ? ui::Glyph::pause : ui::Glyph::play; play.setName(model.playing() ? "Pause" : "Play"); play.setToggleState(model.playing(),juce::dontSendNotification); loop.setToggleState(model.loop,juce::dontSendNotification);
        browserToggle.setToggleState(showBrowser,juce::dontSendNotification); mixerToggle.setToggleState(showMixer,juce::dontSendNotification);
        headers.refresh(); patterns.refresh(); editor.piano=dock==2; editor.refresh(); mode.setButtonText(model.patternMode?"PAT":"SONG");
        if (channels.size()!=static_cast<int>(model.tracks().size())) { channels.clear(); for (int i=0;i<static_cast<int>(model.tracks().size());++i) { auto* c=channels.add(new Channel(model,i)); c->changed=[this]{refresh();}; mixerContent.addAndMakeVisible(c); c->sendLookAndFeelChange(); } }
        for (auto* c:channels) c->refresh();
        wireShortcuts(owner); updatePosition(); owner.resized(); owner.repaint(); canvas.repaint();
    }
    void updatePosition() { const double p=model.position(); position.setText(number(static_cast<int>(p)/4+1)+" : "+number(static_cast<int>(p)%4+1)+" : "+number(static_cast<int>((p-std::floor(p))*100)),juce::dontSendNotification); }
    void timerCallback() override { const auto now=juce::Time::getMillisecondCounterHiRes(); const bool wasPlaying=model.playing(); model.advance((now-previousTime)/1000.0); previousTime=now; if (wasPlaying) { updatePosition(); canvas.repaint(); owner.repaint(rulerArea); if (!model.playing()) refresh(); } }
    void layout() {
        auto area=owner.getLocalBounds(); toolbarArea=area.removeFromTop(62); statusArea=area.removeFromBottom(24);
        file.setBounds(12,15,32,32); title.setBounds(52,15,139,32); undo.setBounds(201,15,30,32); redo.setBounds(237,15,30,32);
        stop.setBounds(291,15,32,32); play.setBounds(329,15,36,32); record.setBounds(371,15,32,32); loop.setBounds(415,15,32,32);
        position.setBounds(462,13,134,36); tempo.setBounds(614,15,46,32); snap.setBounds(769,15,93,32);
        mode.setBounds(705,15,53,32);
        const int right=owner.getWidth(); browserToggle.setBounds(right-126,15,32,32); mixerToggle.setBounds(right-88,15,32,32); help.setBounds(right-50,15,32,32);
        browserArea=showBrowser && !playlistMax ? area.removeFromLeft(std::min(browserWidth,owner.getWidth()/4)) : juce::Rectangle<int>{}; browser.setVisible(!browserArea.isEmpty()); if(!browserArea.isEmpty())browser.setBounds(browserArea);
        const auto patternArea=showPatterns && !playlistMax?area.removeFromRight(patternWidth):juce::Rectangle<int>{};patterns.setVisible(!patternArea.isEmpty());if(!patternArea.isEmpty())patterns.setBounds(patternArea);
        mixerArea=showMixer && !playlistMax ? area.removeFromBottom(std::clamp(dockHeight,210,std::max(210,area.getHeight()/2))) : juce::Rectangle<int>{}; arrangementArea=area;
        browserGrip.setVisible(!browserArea.isEmpty());browserGrip.setBounds(browserArea.getRight()-3,browserArea.getY(),7,browserArea.getHeight());
        patternGrip.setVisible(!patternArea.isEmpty());patternGrip.setBounds(patternArea.getX()-3,patternArea.getY(),7,patternArea.getHeight());
        dockGrip.setVisible(!mixerArea.isEmpty());dockGrip.setBounds(mixerArea.getX(),mixerArea.getY()-3,mixerArea.getWidth(),7);
        auto top=area.removeFromTop(44); addTrack.setBounds(top.getX()+14,top.getY()+8,87,29); addClip.setBounds(top.getX()+107,top.getY()+8,93,29);
        duplicate.setBounds(top.getX()+212,top.getY()+8,30,29); remove.setBounds(top.getX()+248,top.getY()+8,30,29);
        zoomOut.setBounds(top.getRight()-75,top.getY()+8,27,29); zoomIn.setBounds(top.getRight()-42,top.getY()+8,27,29);
        tool.setBounds(top.getX()+286,top.getY()+8,114,29);
        rulerArea=area.removeFromTop(30); headers.setBounds(area.removeFromLeft(144)); lanes.setBounds(area);
        canvas.beatWidth=std::max(8.0,(lanes.getWidth()-10)/Workspace::duration)*zoom;
        canvas.setSize(static_cast<int>(std::ceil(Workspace::duration*canvas.beatWidth))+4,std::max(lanes.getHeight()-10,static_cast<int>(model.tracks().size())*rowHeight));
        headers.scroll=lanes.getViewPositionY(); headers.resized();
        mixerView.setVisible(!mixerArea.isEmpty() && dock==0);editor.setVisible(!mixerArea.isEmpty() && dock!=0);if(editor.isVisible())editor.setBounds(mixerArea);
        if(!mixerArea.isEmpty() && dock==0) { mixerView.setBounds(mixerArea.withTrimmedTop(32).withTrimmedLeft(84)); mixerContent.setSize(std::max(mixerView.getWidth(),channels.size()*80),mixerView.getHeight()-7); for(int n=0;n<channels.size();++n) channels[n]->setBounds(n*80,0,80,mixerContent.getHeight()); }
        rack.setBounds(14,statusArea.getY(),74,23);piano.setBounds(94,statusArea.getY(),74,23);picker.setBounds(174,statusArea.getY(),92,23);preview.setBounds(owner.getWidth()-102,statusArea.getY(),91,23); helpPanel.setBounds(owner.getLocalBounds()); helpPanel.setVisible(helpOpen);
    }
    void paint(juce::Graphics& g) {
        g.fillAll(bg); g.setColour(panel); g.fillRect(toolbarArea); g.setColour(line); g.drawHorizontalLine(toolbarArea.getBottom()-1,0,static_cast<float>(owner.getWidth()));
        g.drawVerticalLine(280,18,44); g.drawVerticalLine(603,18,44);
        ui::label(g,"bpm",{666,17,32,29},14,muted);
        if(!mixerArea.isEmpty() && dock==0) {
            g.setColour(panel); g.fillRect(mixerArea); g.setColour(line); g.drawHorizontalLine(mixerArea.getY(),static_cast<float>(mixerArea.getX()),static_cast<float>(mixerArea.getRight()));
            ui::label(g,"Mixer",{mixerArea.getX()+96,mixerArea.getY(),120,29},14,muted);
            const int x=mixerArea.getX()+9, y=mixerArea.getY()+34;
            ui::glyph(g,ui::Glyph::speaker,{static_cast<float>(x),static_cast<float>(y+2),17,17},muted); ui::label(g,"Master",{x+3,y+22,70,23},12);
            g.setColour(bg); g.fillRoundedRectangle(static_cast<float>(x+15),static_cast<float>(y+62),5,static_cast<float>(std::max(32,mixerArea.getHeight()-145)),2); g.fillRoundedRectangle(static_cast<float>(x+28),static_cast<float>(y+62),5,static_cast<float>(std::max(32,mixerArea.getHeight()-145)),2);
            ui::label(g,"-inf",{x+5,mixerArea.getBottom()-31,68,24},14,muted);
        }
        g.setColour(panel); g.fillRect(rulerArea); ui::label(g,"Tracks",rulerArea.withWidth(144).reduced(16,0),14,muted);
        const int origin=lanes.getX()-lanes.getViewPositionX();
        { juce::Graphics::ScopedSaveState saved(g); g.reduceClipRegion(rulerArea.withTrimmedLeft(144));
          for(int bar=0;bar<16;++bar) { const int x=origin+static_cast<int>(bar*4*canvas.beatWidth); ui::label(g,juce::String(bar+1),{x+6,rulerArea.getY(),45,30},12,bar%4==0 ? ink:muted); }
          const int px=origin+static_cast<int>(model.position()*canvas.beatWidth); g.setColour(ink); g.fillRoundedRectangle(static_cast<float>(px),static_cast<float>(rulerArea.getY()+22),5,7,1);
        }
        g.setColour(panel); g.fillRect(statusArea); ui::label(g,juce::String(static_cast<int>(model.tracks().size()))+" lanes  /  "+juce::String(static_cast<int>(model.patterns().size()))+" patterns",statusArea.withTrimmedLeft(282).withWidth(250),14,muted);
    }
};

WorkspaceView::WorkspaceView() { impl_=std::make_unique<Impl>(*this); setSize(1440,900); impl_->refresh(); }
WorkspaceView::~WorkspaceView()=default;
void WorkspaceView::paint(juce::Graphics& g) { impl_->paint(g); }
void WorkspaceView::resized() { if(impl_) impl_->layout(); }
const Workspace& WorkspaceView::workspace() const noexcept { return impl_->model; }
void WorkspaceView::perform(ShellAction action) {
    auto& i=*impl_;
    switch(action) {
        case ShellAction::playPause:i.model.togglePlay();break;
        case ShellAction::stop:i.model.stop();break;
        case ShellAction::newSession:i.model.newSession();break;
        case ShellAction::demo:i.model.resetDemo();break;
        case ShellAction::addTrack:i.model.addTrack("Track "+std::to_string(i.model.tracks().size()+1));break;
        case ShellAction::addClip:if(i.model.activePattern()==0)i.model.addPattern("Pattern 1");i.model.placePattern(i.model.selectedTrack(),std::min(i.model.position(),60.0));break;
        case ShellAction::remove:if(i.model.selectedClip()!=0)i.model.removeClip();else i.model.removeTrack();break;
        case ShellAction::duplicate:i.model.duplicateClip();break;
        case ShellAction::undo:i.model.undo();break;
        case ShellAction::redo:i.model.redo();break;
        case ShellAction::browser:i.showBrowser=!i.showBrowser;break;
        case ShellAction::mixer:if(i.dock==0)i.showMixer=!i.showMixer;else{i.dock=0;i.showMixer=true;}break;
        case ShellAction::help:i.helpOpen=!i.helpOpen;break;
    }
    i.refresh();
    if(action==ShellAction::help) { if(i.helpOpen)i.helpPanel.close.grabKeyboardFocus();else grabKeyboardFocus(); }
}
bool WorkspaceView::keyPressed(const juce::KeyPress& key) {
    auto& i=*impl_;const int c=key.getKeyCode();const auto mods=key.getModifiers();
    if(dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent())!=nullptr)return false;
    if(i.helpOpen){if(c==juce::KeyPress::escapeKey)perform(ShellAction::help);return true;}
    const bool playlistContext=!(i.editor.hasKeyboardFocus(true)||i.mixerView.hasKeyboardFocus(true)||i.browser.hasKeyboardFocus(true)||i.patterns.hasKeyboardFocus(true));
    if(mods.isAltDown()){
        if(c==juce::KeyPress::F8Key){perform(ShellAction::browser);return true;}
        if(c=='P'){i.showPatterns=!i.showPatterns;i.refresh();return true;}
        return false;
    }
    if(c==juce::KeyPress::spaceKey){if(mods.isCtrlDown())i.model.togglePlay();else if(i.model.playing())i.model.stop();else i.model.togglePlay();i.refresh();return true;}
    if(mods.isCtrlDown()){
        if(c=='F'){i.showBrowser=true;i.playlistMax=false;i.refresh();i.browser.search.grabKeyboardFocus();return true;}
        if(c=='Z')perform(ShellAction::undo);else if(c=='Y')perform(ShellAction::redo);
        else if(c=='B' && playlistContext)perform(ShellAction::duplicate);
        else if(c=='D' && playlistContext){i.model.deselectClip();i.refresh();}
        else if(c=='S' || c=='N')perform(ShellAction::help); // Unavailable save/version, never reset or claim save.
        else return false;
        return true;
    }
    if(c==juce::KeyPress::F4Key){if(i.model.addPattern("Pattern "+std::to_string(i.model.patterns().size()+1))){i.showPatterns=true;i.refresh();i.patterns.beginRename();}return true;}
    if(c==juce::KeyPress::F2Key){if(i.mixerView.hasKeyboardFocus(true) && i.model.selectedTrack()>=0)i.headers.names[i.model.selectedTrack()]->showEditor();else {i.showPatterns=true;i.playlistMax=false;i.refresh();i.patterns.beginRename();}return true;}
    if(c==juce::KeyPress::F5Key){i.playlistMax=false;i.canvas.grabKeyboardFocus();i.refresh();return true;}
    if(c==juce::KeyPress::F6Key){i.rack.onClick();return true;}
    if(c==juce::KeyPress::F7Key){i.piano.onClick();return true;}
    if(c==juce::KeyPress::F9Key){perform(ShellAction::mixer);return true;}
    if(c==juce::KeyPress::returnKey){i.playlistMax=!i.playlistMax;i.refresh();return true;}
    if(c==juce::KeyPress::homeKey){i.model.stop();i.refresh();return true;}
    if(c==juce::KeyPress::deleteKey && i.model.selectedClip()!=0 && playlistContext){i.model.removeClip();i.refresh();return true;}
    if(c=='L'){i.mode.onClick();return true;}
    if(c=='?' || c==juce::KeyPress::F1Key){perform(ShellAction::help);return true;}
    if(c=='S' && i.mixerView.hasKeyboardFocus(true)){i.model.toggleSolo();i.refresh();return true;}
    if(!playlistContext)return false;
    const juce::String keys="PBEDTCZ";const int index=keys.indexOfChar(static_cast<juce::juce_wchar>(c));
    if(index>=0){i.tool.setSelectedId(index+1,juce::sendNotificationSync);return true;}
    return false;
}
bool WorkspaceView::keyPressed(const juce::KeyPress& key,juce::Component* source) {
    if(key.getKeyCode()==juce::KeyPress::returnKey&&source!=this)return false;
    return keyPressed(key);
}
juce::String WorkspaceView::runSmokeTest() {
    auto& i=*impl_;
    const auto clickIn=[](juce::Component& parent,const juce::String& id) {
        std::function<juce::Component*(juce::Component&)> find=[&](juce::Component& c)->juce::Component* {
            if(c.getComponentID()==id)return &c;
            for(int n=0;n<c.getNumChildComponents();++n)if(auto* found=find(*c.getChildComponent(n)))return found;
            return nullptr;
        };
        auto* button=dynamic_cast<juce::Button*>(find(parent));
        if(button==nullptr || !button->isEnabled() || !button->isVisible() || !button->onClick)return false;
        button->onClick();return true;
    };
    const auto click=[&](const char* id){return clickIn(*this,id);};
    if(i.model.tracks().size()!=4)return "Initial demo missing";
    perform(ShellAction::newSession);
    if(!i.model.tracks().empty() || i.addClip.isEnabled() || i.remove.isEnabled())return "Empty session controls failed";
    if(!click("add-track") || !click("add-clip") || i.model.tracks().size()!=1 || i.model.selectedClip()==0)return "Native add controls failed";
    if(!click("duplicate") || i.model.tracks()[0].clips.size()!=2 || !click("remove") || i.model.tracks()[0].clips.size()!=1)return "Clip controls failed";
    i.headers.names[0]->showEditor();
    auto* nameEditor=i.headers.names[0]->getCurrentTextEditor();
    if(nameEditor==nullptr)return "Inline track editor missing";
    nameEditor->setText("Renamed",false); i.headers.names[0]->hideEditor(false);
    if(i.model.tracks()[0].name!="Renamed")return "Inline track rename failed";
    i.tempo.setText("0",false); i.tempo.onReturnKey(); if(i.model.bpm()!=120)return "Invalid tempo accepted";
    i.tempo.setText("96",false); i.tempo.onReturnKey(); if(i.model.bpm()!=96)return "Tempo edit failed";
    i.headers.mute[0]->onClick(); i.headers.solo[0]->onClick();
    if(!i.model.tracks()[0].muted || !i.model.tracks()[0].solo)return "Track mute/solo failed";
    i.channels[0]->level.setValue(-12,juce::sendNotificationSync);
    if(i.model.tracks()[0].level!=-12)return "Mixer level control failed";
    i.channels[0]->pan.setValue(-0.25,juce::sendNotificationSync);if(i.model.tracks()[0].pan!=-0.25)return "Mixer pan binding failed";
    juce::Label* levelText=nullptr;
    for(int n=0;n<i.channels[0]->level.getNumChildComponents();++n)
        if(auto* candidate=dynamic_cast<juce::Label*>(i.channels[0]->level.getChildComponent(n)))levelText=candidate;
    if(levelText==nullptr)return "Editable mixer level field missing";
    if(levelText->findColour(juce::Label::outlineColourId)!=juce::Colours::transparentBlack || levelText->getFont().getHeight()!=14 || levelText->findColour(juce::TextEditor::highlightColourId)!=ui::selected)return "Cached mixer textbox theme drifted";
    levelText->showEditor();
    if(levelText->getCurrentTextEditor()==nullptr)return "Mixer level text editor missing";
    levelText->getCurrentTextEditor()->setText("-9.5 dB",false); levelText->hideEditor(false);
    if(std::abs(i.model.tracks()[0].level+9.5)>0.001)return "Mixer dB text entry failed";
    for(const double value:{-60.0,6.0}) { i.channels[0]->level.setValue(value,juce::sendNotificationSync); if(i.model.tracks()[0].level!=value)return "Mixer level boundary failed"; }
    i.channels[0]->level.setValue(-12,juce::sendNotificationSync);
    i.channels[0]->mute.onClick(); i.channels[0]->solo.onClick();
    if(i.model.tracks()[0].muted || i.model.tracks()[0].solo)return "Mixer mute/solo failed";
    i.snap.setSelectedId(1,juce::sendNotificationSync); if(i.model.grid!=4)return "Snap control failed";
    const auto oldRoot=i.browser.rootDirectory();
    const auto root=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("simplicity-browser-test",{},false);
    if(root.createDirectory().failed() || !root.getChildFile("Kick One.WAV").replaceWithText("fixture") || !root.getChildFile("lead.mid").replaceWithText("fixture") || !root.getChildFile("notes.txt").replaceWithText("fixture") || root.getChildFile("Drums").createDirectory().failed())return "Browser fixture creation failed";
    const auto waitBrowser=[&]{for(int n=0;n<500 && i.browser.isScanning();++n)juce::Thread::sleep(10);return !i.browser.isScanning();};
    if(!i.browser.setRootDirectory(root) || !waitBrowser() || i.browser.visibleFileCount()!=4)return "Asynchronous filesystem listing failed";
    i.browser.setCategory(2);if(!waitBrowser() || i.browser.visibleFileCount()!=2 || !i.browser.acceptsFile(root.getChildFile("Kick One.WAV")) || i.browser.acceptsFile(root.getChildFile("lead.mid")))return "Audio file filter failed";
    i.browser.setCategory(3);if(!waitBrowser() || i.browser.visibleFileCount()!=2 || !i.browser.acceptsFile(root.getChildFile("lead.mid")))return "MIDI file filter failed";
    i.browser.setCategory(1);i.browser.setQuery(" kIcK ONE ");if(!waitBrowser() || i.browser.visibleFileCount()!=2)return "Case-insensitive filename filter failed";
    i.browser.setQuery("nonexistent");if(!waitBrowser() || i.browser.visibleFileCount()!=1)return "Filtered folders must stay navigable";
    if(i.browser.setRootDirectory(root.getChildFile("missing")) || i.browser.rootDirectory()!=root)return "Invalid browser root accepted";
    i.browser.search.grabKeyboardFocus();
    if(keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)) || i.model.playing() || keyPressed(juce::KeyPress('B')))return "Search shortcut isolation failed";
    grabKeyboardFocus();i.browser.setQuery({});i.browser.setRootDirectory(oldRoot);root.deleteRecursively();
    if(keyPressed(juce::KeyPress('I')) || findChildWithID("inspector")!=nullptr)return "Removed inspector is still reachable";
    if(!keyPressed(juce::KeyPress(juce::KeyPress::F8Key,juce::ModifierKeys::altModifier,0)) || i.showBrowser || !keyPressed(juce::KeyPress(juce::KeyPress::F8Key,juce::ModifierKeys::altModifier,0)) || !i.showBrowser)return "Browser shortcut failed";
    if(!keyPressed(juce::KeyPress('B')) || i.canvas.tool!=Canvas::Tool::paint || !keyPressed(juce::KeyPress('P')) || i.canvas.tool!=Canvas::Tool::draw)return "Contextual tool shortcuts failed";
    perform(ShellAction::mixer);if(i.mixerView.isVisible())return "Mixer hide failed";perform(ShellAction::mixer);
    if(!keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)) || !i.model.playing())return "Visual play failed";
    if(!keyPressed(juce::KeyPress(juce::KeyPress::spaceKey,juce::ModifierKeys::ctrlModifier,0)) || i.model.playing())return "Visual pause failed";
    if(!keyPressed(juce::KeyPress('L')) || !i.model.patternMode)return "PAT/SONG shortcut failed";
    keyPressed(juce::KeyPress('L'));
    perform(ShellAction::stop);perform(ShellAction::undo);perform(ShellAction::redo);perform(ShellAction::help);
    if(!i.helpPanel.isVisible())return "Help failed";
    if(!keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)) || i.helpPanel.isVisible())return "Help Escape failed";
    for(const auto size:{juce::Point<int>{1080,640},juce::Point<int>{1080,700},juce::Point<int>{1440,900},juce::Point<int>{1920,1080}}){
        setSize(size.x,size.y);
        if(i.toolbarArea.getHeight()!=62 || i.lanes.getWidth()<300 || i.lanes.getHeight()<100 || i.patterns.getRight()!=getWidth() || i.patterns.getX()<i.lanes.getRight() || i.addTrack.getBounds().intersects(i.zoomOut.getBounds()) || i.tool.getBounds().intersects(i.zoomOut.getBounds()) || i.snap.getRight()>i.browserToggle.getX())return "Responsive dock layout failed";
        if(!createComponentSnapshot(getLocalBounds()).isValid())return "Component render failed";
    }
    // Track cap/scrolling and undo remain independent of filesystem discovery.
    perform(ShellAction::newSession);
    for(int n=0;n<Workspace::maxTracks;++n)if(!click("add-track"))return "Track creation failed";
    if(i.addTrack.isEnabled() || i.model.tracks().size()!=Workspace::maxTracks)return "Track limit not respected";
    i.lanes.setViewPosition(0,23*rowHeight);if(i.lanes.getViewPositionY()==0 || i.headers.scroll==0)return "Playlist/header scroll synchronization failed";
    perform(ShellAction::remove);if(i.model.tracks().size()!=23)return "Track deletion failed";
    perform(ShellAction::undo);if(i.model.tracks().size()!=24)return "Track undo failed";
    perform(ShellAction::demo);
    const auto firstId=i.model.tracks()[0].clips[0].id;
    const auto down=i.canvas.clipBounds(i.model.tracks()[0].clips[0],0).getCentre();
    auto eventAt=[&i,down](juce::Point<float> point) {
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,juce::ModifierKeys::leftButtonModifier,1.0F,0,0,0,0,&i.canvas,&i.canvas,juce::Time::getCurrentTime(),down,juce::Time::getCurrentTime(),1,true);
    };
    i.canvas.mouseDown(eventAt(down)); const auto destination=down.translated(static_cast<float>(4*i.canvas.beatWidth),0);
    i.canvas.mouseDrag(eventAt(destination)); i.canvas.mouseUp(eventAt(destination));
    if(i.model.clip(firstId)->beat!=4 || i.model.selectedClip()!=firstId)return "Clip drag binding failed";
    perform(ShellAction::undo); if(i.model.clip(firstId)->beat!=0)return "Clip drag undo failed";
    perform(ShellAction::newSession);click("add-track");
    if(!click("new-pattern") || i.model.patterns().size()!=1)return "Pattern picker create failed";
    const int shared=i.model.activePattern();
    if(!i.editor.editCell(0,0) || i.model.pattern(shared)->notes.size()!=1 || !click("add-clip"))return "Step editor/placement binding failed";
    if(!click("duplicate") || !click("unique-pattern") || i.model.activePattern()==shared || i.model.patterns().size()!=2)return "Make-unique binding failed";
    const int isolated=i.model.activePattern();
    if(!i.editor.editCell(0,0) || !i.model.pattern(i.model.activePattern())->notes.empty() || i.model.pattern(shared)->notes.size()!=1)return "Shared-source isolation failed";
    if(!click("clone-pattern") || !click("delete-pattern") || i.model.patterns().size()!=2)return "Unused source clone/delete failed";
    i.model.selectPattern(isolated);i.refresh();
    grabKeyboardFocus();if(!keyPressed(juce::KeyPress(juce::KeyPress::F7Key)) || !i.editor.piano || i.dock!=2)return "Piano shortcut failed";
    i.editor.basePitch=48;const auto r=i.editor.getLocalBounds().withTrimmedTop(43).withTrimmedLeft(86).reduced(0,8).withTrimmedRight(14);const float cell=static_cast<float>(r.getWidth())/16;
    const juce::Point<float> noteStart{static_cast<float>(r.getX())+cell*0.5F,static_cast<float>(r.getY()+r.getHeight()/24)};
    const auto editorEvent=[&](juce::Point<float> point,int mods){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),point,juce::ModifierKeys(mods),1.0F,0,0,0,0,&i.editor,&i.editor,juce::Time::getCurrentTime(),noteStart,juce::Time::getCurrentTime(),1,true);};
    i.editor.mouseDown(editorEvent(noteStart,juce::ModifierKeys::leftButtonModifier));i.editor.mouseUp(editorEvent(noteStart.translated(2*cell,0),juce::ModifierKeys::leftButtonModifier));
    const auto* edited=i.model.pattern(i.model.activePattern());if(edited->notes.size()!=1 || edited->notes[0].length!=72)return "Piano drag duration failed";
    i.editor.mouseDown(editorEvent(noteStart,juce::ModifierKeys::leftButtonModifier));i.editor.mouseUp(editorEvent(noteStart.translated(3*cell,0),juce::ModifierKeys::leftButtonModifier));
    if(i.model.pattern(i.model.activePattern())->notes[0].length!=96)return "Piano note resize failed";
    i.editor.mouseDown(editorEvent(noteStart.translated(cell,0),juce::ModifierKeys::rightButtonModifier));i.editor.mouseUp(editorEvent(noteStart,juce::ModifierKeys::rightButtonModifier));
    if(!i.model.pattern(i.model.activePattern())->notes.empty())return "Piano sustained-note erasure failed";
    if(keyPressed(juce::KeyPress('B')))return "Piano must not change playlist tools";
    grabKeyboardFocus();if(!keyPressed(juce::KeyPress(juce::KeyPress::F6Key)) || i.editor.piano)return "Rack shortcut failed";
    grabKeyboardFocus();keyPressed(juce::KeyPress(juce::KeyPress::F4Key));
    if(i.model.patterns().size()!=3)return "F4 pattern creation failed";
    // Inline source editor must retain normal text shortcuts.
    if(keyPressed(juce::KeyPress('L')))return "Pattern rename shortcut isolation failed";
    i.patterns.endRename();grabKeyboardFocus();perform(ShellAction::demo);i.dock=0;
    perform(ShellAction::demo); i.snap.setSelectedId(3,juce::sendNotificationSync); i.zoom=1; setSize(1440,900); i.refresh();
    return {};
}
} // namespace simplicity
