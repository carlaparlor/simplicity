#include <simplicity/workspace_view.h>
#include <simplicity/ui_style.h>
#include <simplicity/browser_panel.h>
#include <algorithm>
#include <cmath>
#include <functional>

namespace simplicity {
namespace {
const auto bg = ui::background, panel = ui::panel, line = ui::border, ink = ui::foreground, muted = ui::secondary;
const juce::Colour trackColours[]{juce::Colour{0xffa4c98e}, juce::Colour{0xffcba57e}, juce::Colour{0xff8daabf}, juce::Colour{0xffb1a0c9}};
constexpr int rowHeight = 80;
juce::String number(int n) { return juce::String(n).paddedLeft('0', 2); }
class Canvas final : public juce::Component {
public:
    Workspace& model;
    std::function<void()> changed;
    double beatWidth{12};
    int dragged{}, dragRow{-1}; double dragStart{}, preview{};
    explicit Canvas(Workspace& m) : model(m) { setName("Arrangement sketch clips. Drag horizontally to move; double-click an empty lane to add."); }
    juce::Rectangle<float> clipBounds(const SketchClip& c, int row) const {
        const double beat = dragged == c.id ? preview : c.beat;
        return {static_cast<float>(beat * beatWidth + 2), static_cast<float>(row * rowHeight + 10),
                static_cast<float>(c.length * beatWidth - 4), static_cast<float>(rowHeight - 20)};
    }
    void paint(juce::Graphics& g) override {
        g.fillAll(bg);
        for (int bar = 0; bar < 16; ++bar) {
            const int x = static_cast<int>(bar * 4 * beatWidth);
            if ((bar / 4) % 2 == 0) { g.setColour(juce::Colour{0xff1b1b1b}); g.fillRect(x, 0, static_cast<int>(4 * beatWidth), getHeight()); }
            g.setColour(line.withAlpha(0.7F)); g.drawVerticalLine(x, 0, static_cast<float>(getHeight()));
            for (int beat = 1; beat < 4; ++beat) { g.setColour(line.withAlpha(0.25F)); g.drawVerticalLine(x + static_cast<int>(beat * beatWidth), 0, static_cast<float>(getHeight())); }
        }
        const auto& tracks = model.tracks();
        for (std::size_t row = 0; row < tracks.size(); ++row) {
            const auto& track = tracks[row]; const auto colour = trackColours[track.colour % 4];
            const int y = static_cast<int>(row) * rowHeight;
            g.setColour(line); g.drawHorizontalLine(y + rowHeight - 1, 0, static_cast<float>(getWidth()));
            if (!g.getClipBounds().intersects(juce::Rectangle<int>{0, y, getWidth(), rowHeight})) continue;
            for (const auto& c : track.clips) {
                const auto r = clipBounds(c, static_cast<int>(row));
                if (!g.getClipBounds().intersects(r.toNearestInt())) continue;
                g.setColour(colour.withAlpha(track.muted ? 0.12F : 0.27F)); g.fillRoundedRectangle(r, 3);
                g.setColour(colour.withAlpha(track.muted ? 0.3F : 0.85F)); g.fillRect(r.getX(), r.getY(), r.getWidth(), 3.0F);
                if (model.selectedClip() == c.id) { g.setColour(ink); g.drawRoundedRectangle(r, 3, 1.5F); }
                ui::label(g, juce::String(c.name), r.toNearestInt().reduced(8, 4).withHeight(21), 11, colour, true);
                // Deliberately illustrative note blocks, not audio waveforms or recorded data.
                g.setColour(colour.withAlpha(track.muted ? 0.15F : 0.65F));
                for (int step = 0; step < static_cast<int>(c.length); ++step) {
                    const float px = r.getX() + 8 + static_cast<float>(step * beatWidth);
                    if (px + 5 < r.getRight() - 6) g.fillRect(px, r.getY() + 33 + static_cast<float>((step * 7 + static_cast<int>(row) * 3) % 3) * 5, static_cast<float>(std::max(3.0, beatWidth * 0.6)), 3.0F);
                }
            }
        }
        if (tracks.empty()) ui::label(g, "No tracks", {28, 36, getWidth()-56, 30}, 16, muted);
        const auto x = static_cast<float>(model.position() * beatWidth);
        g.setColour(ink); g.fillRect(x, 0.0F, 1.5F, static_cast<float>(getHeight()));
    }
    void mouseDown(const juce::MouseEvent& e) override {
        dragged = 0; dragRow = -1;
        for (std::size_t row = 0; row < model.tracks().size(); ++row)
            for (const auto& c : model.tracks()[row].clips) if (clipBounds(c, static_cast<int>(row)).contains(e.position)) {
                model.selectClip(c.id); dragged = c.id; dragRow = static_cast<int>(row); dragStart = c.beat; preview = c.beat; changed(); return;
            }
        model.selectTrack(e.y / rowHeight); model.seek(e.position.x / beatWidth); changed();
    }
    void mouseDrag(const juce::MouseEvent& e) override {
        if (const auto* c = model.clip(dragged)) { preview = std::clamp(Workspace::snapped(dragStart + e.getDistanceFromDragStartX() / beatWidth, model.grid), 0.0, Workspace::duration - c->length); repaint(); }
    }
    void mouseUp(const juce::MouseEvent&) override { if (dragged != 0) { const auto id = dragged; dragged = 0; model.moveClip(id, preview); changed(); } }
    void mouseDoubleClick(const juce::MouseEvent& e) override { if (model.selectedClip() == 0 && model.selectTrack(e.y / rowHeight)) { model.addClip(); changed(); } }
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
    TrackName() { setEditable(false, true); setFont(ui::font(13)); setBorderSize({0, 8, 0, 6}); }
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
    void resized() override { for (int i = 0; i < names.size(); ++i) { const int y = i*rowHeight-scroll; names[i]->setBounds(30,y+10,getWidth()-42,27); mute[i]->setBounds(30,y+46,26,23); solo[i]->setBounds(62,y+46,26,23); } }
    void paint(juce::Graphics& g) override {
        g.fillAll(panel);
        for (int i = 0; i < names.size(); ++i) {
            const int y = i*rowHeight-scroll; const auto colour = trackColours[model.tracks()[static_cast<std::size_t>(i)].colour % 4];
            ui::glyph(g, ui::Glyph::wave, {8, static_cast<float>(y+15), 16, 16}, colour);
            ui::label(g, number(i+1), {8,y+46,20,23}, 10, muted);
            g.setColour(line.withAlpha(0.5F)); g.drawHorizontalLine(y+rowHeight-1, 0, static_cast<float>(getWidth()));
        }
    }
};
class Channel final : public juce::Component {
public:
    Workspace& model; int index; std::function<void()> changed;
    juce::TextButton select, mute{"M"}, solo{"S"}; juce::Slider level;
    Channel(Workspace& m, int i) : model(m), index(i) {
        for (auto* c : std::initializer_list<juce::Component*>{&select,&mute,&solo,&level}) addAndMakeVisible(c);
        level.setSliderStyle(juce::Slider::LinearVertical); level.setTextBoxStyle(juce::Slider::TextBoxBelow,false,82,21); level.setRange(-60,6,0.1); level.setTextValueSuffix(" dB"); level.setDoubleClickReturnValue(true,-6);
        level.setName("Track level, dB"); level.setTooltip("Track level (visual sketch only)");
        level.onDragEnd = [this] { model.selectTrack(index); model.setLevel(level.getValue()); changed(); };
        level.onValueChange = [this] { if (!level.isMouseButtonDown()) { model.selectTrack(index); model.setLevel(level.getValue()); if (changed) changed(); } };
        select.onClick = [this] { model.selectTrack(index); changed(); };
        mute.onClick = [this] { model.selectTrack(index); model.toggleMute(); changed(); };
        solo.onClick = [this] { model.selectTrack(index); model.toggleSolo(); changed(); };
        mute.setTooltip("Mute (sketch state)"); solo.setTooltip("Solo (sketch state)");
    }
    void refresh() { const auto& t = model.tracks()[static_cast<std::size_t>(index)]; select.setButtonText(t.name); select.setToggleState(model.selectedTrack()==index,juce::dontSendNotification); mute.setToggleState(t.muted,juce::dontSendNotification); solo.setToggleState(t.solo,juce::dontSendNotification); if (!level.isMouseButtonDown()) level.setValue(t.level,juce::dontSendNotification); repaint(); }
    void resized() override { select.setBounds(8,9,getWidth()-16,26); mute.setBounds(15,41,26,22); solo.setBounds(47,41,26,22); level.setBounds(8,69,82,getHeight()-73); }
    void paint(juce::Graphics& g) override {
        g.fillAll(panel); g.setColour(line.withAlpha(0.5F)); g.drawVerticalLine(getWidth()-1,0,static_cast<float>(getHeight()));
        g.setColour(trackColours[model.tracks()[static_cast<std::size_t>(index)].colour % 4]); g.fillRoundedRectangle(9,0,static_cast<float>(getWidth()-18),2,1);
        g.setColour(bg); g.fillRoundedRectangle(static_cast<float>(getWidth()-21),75,4,static_cast<float>(std::max(1,getHeight()-110)),2);
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
        ui::label(g,"Shortcuts",r.removeFromTop(42),22,ink,true);
        for (const auto& s : {"Space     Play / pause", "Enter      Stop and return", "Ctrl N     New session      Ctrl Z / Y     Undo / redo", "Ctrl D     Duplicate clip    Delete     Delete selection", "B / M      Browser / mixer", "Double-click a track name to rename it."}) ui::label(g,s,r.removeFromTop(31),14);
        ui::label(g,"Preview: visual sketches only.",r.removeFromTop(40),13,muted);
        ui::label(g,"Audio, recording and project saving are not available yet.",r.removeFromTop(28),13,muted);
    }
};
} // namespace

class WorkspaceView::Impl final : private juce::Timer {
public:
    WorkspaceView& owner; Workspace model; ui::Look look;
    juce::TooltipWindow tooltips{&owner,500};
    juce::Label title, position;
    juce::TextEditor tempo;
    ui::IconButton file{"Session menu",ui::Glyph::folder}, play{"Play",ui::Glyph::play}, stop{"Stop",ui::Glyph::stop}, record{"Record",ui::Glyph::record}, undo{"Undo",ui::Glyph::undo}, redo{"Redo",ui::Glyph::redo};
    ui::IconButton browserToggle{"Browser",ui::Glyph::tracks}, mixerToggle{"Mixer",ui::Glyph::mixer}, loop{"Loop",ui::Glyph::loop}, help{"Help",ui::Glyph::help};
    ui::IconButton addTrack{"Track",ui::Glyph::plus,true}, addClip{"Sketch",ui::Glyph::pattern,true}, duplicate{"Duplicate clip",ui::Glyph::duplicate}, remove{"Delete selection",ui::Glyph::trash}, zoomOut{"Zoom out",ui::Glyph::minus}, zoomIn{"Zoom in",ui::Glyph::plus};
    ui::IconButton preview{"Preview",ui::Glyph::help,true};
    juce::ComboBox snap;
    BrowserPanel browser{model};
    Canvas canvas{model}; ScrollingView lanes; TrackHeaders headers{model};
    juce::Component mixerContent; juce::Viewport mixerView;
    juce::OwnedArray<Channel> channels;
    HelpPanel helpPanel;
    bool showBrowser{true}, showMixer{true}, helpOpen{};
    double zoom{1}, previousTime{juce::Time::getMillisecondCounterHiRes()};
    juce::Rectangle<int> toolbarArea, browserArea, arrangementArea, rulerArea, mixerArea, statusArea;
    explicit Impl(WorkspaceView& o) : owner(o) {
        owner.setLookAndFeel(&look); owner.setWantsKeyboardFocus(true); owner.setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer); owner.setName("Simplicity workspace");
        for (auto* c : std::initializer_list<juce::Component*>{&title,&position,&tempo,&file,&play,&stop,&record,&undo,&redo,&browserToggle,&mixerToggle,&loop,&help,&addTrack,&addClip,&duplicate,&remove,&zoomOut,&zoomIn,&preview,&snap,&browser,&lanes,&headers,&mixerView}) owner.addAndMakeVisible(c);
        owner.addChildComponent(helpPanel);
        title.setFont(ui::font(14,true)); title.setEditable(false,true); title.setTooltip("Double-click to rename session"); title.onTextChange = [this] { model.setTitle(title.getText().toStdString()); refresh(); };
        position.setFont(ui::font(22)); position.setTooltip("Visual position: bar, beat, fraction");
        tempo.setName("Tempo, 40 to 240 BPM"); tempo.setFont(ui::font(14)); tempo.setInputRestrictions(3,"0123456789"); tempo.setBorder({5,8,4,4});
        tempo.onReturnKey = [this] { model.setBpm(tempo.getText().getDoubleValue()); refresh(); owner.grabKeyboardFocus(); };
        tempo.onFocusLost = [this] { model.setBpm(tempo.getText().getDoubleValue()); refresh(); };
        bind(play,ShellAction::playPause,"Play / pause visual transport (Space)"); play.getProperties().set("primary",true);
        bind(stop,ShellAction::stop,"Return to start (Enter)"); record.setEnabled(false); record.setTooltip("Recording is not available yet");
        bind(undo,ShellAction::undo,"Undo (Ctrl Z)"); bind(redo,ShellAction::redo,"Redo (Ctrl Y)");
        bind(browserToggle,ShellAction::browser,"Browser (B)"); bind(mixerToggle,ShellAction::mixer,"Mixer (M)");
        bind(addTrack,ShellAction::addTrack,"Add an empty track"); bind(addClip,ShellAction::addClip,"Add a four-beat sketch at the playhead");
        bind(duplicate,ShellAction::duplicate,"Duplicate selected clip (Ctrl D)"); bind(remove,ShellAction::remove,"Delete selected clip, or selected track (Delete)");
        bind(help,ShellAction::help,"Help and shortcuts"); bind(preview,ShellAction::help,"Visual preview only: no audio, recording or saving. Open help for details.");
        preview.getProperties().set("flat",true);
        loop.onClick = [this] { model.loop = !model.loop; refresh(); }; loop.setTooltip("Loop the visual 16-bar sketch");
        file.onClick = [this] {
            juce::PopupMenu menu; menu.addItem(1,"New session",true); menu.addItem(2,"Demo session",true); menu.addSeparator(); menu.addItem(3,"Save (not available yet)",false);
            auto safe = juce::Component::SafePointer<WorkspaceView>(&owner);
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&file), [safe](int item) { if (safe != nullptr && item != 0) safe->perform(item==1 ? ShellAction::newSession : ShellAction::demo); });
        };
        snap.addItem("Bar",1); snap.addItem("1/2 beat",2); snap.addItem("1/4 beat",3); snap.addItem("Off",4); snap.setSelectedId(3,juce::dontSendNotification); model.grid = 0.25;
        snap.setName("Clip snap grid"); snap.setTooltip("Clip movement snap grid"); snap.onChange = [this] { const double values[]{4,0.5,0.25,0}; model.grid = values[std::clamp(snap.getSelectedId()-1,0,3)]; };
        zoomOut.onClick = [this] { zoom = std::max(0.75,zoom-0.25); owner.resized(); };
        zoomIn.onClick = [this] { zoom = std::min(3.0,zoom+0.25); owner.resized(); };
        lanes.setViewedComponent(&canvas,false); lanes.setScrollBarsShown(true,true); lanes.setScrollBarThickness(7);
        lanes.scrolled = [this] { headers.scroll=lanes.getViewPositionY(); headers.resized(); headers.repaint(); owner.repaint(rulerArea); };
        canvas.changed = [this] { refresh(); }; headers.changed = [this] { refresh(); }; browser.changed = [this] { refresh(); };
        mixerView.setViewedComponent(&mixerContent,false); mixerView.setScrollBarsShown(false,true); mixerView.setScrollBarThickness(7);
        helpPanel.close.onClick = [this] { owner.perform(ShellAction::help); };
        setIDs(); startTimerHz(30);
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
        headers.refresh(); browser.refresh();
        if (channels.size()!=static_cast<int>(model.tracks().size())) { channels.clear(); for (int i=0;i<static_cast<int>(model.tracks().size());++i) { auto* c=channels.add(new Channel(model,i)); c->changed=[this]{refresh();}; mixerContent.addAndMakeVisible(c); } }
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
        const int right=owner.getWidth(); browserToggle.setBounds(right-126,15,32,32); mixerToggle.setBounds(right-88,15,32,32); help.setBounds(right-50,15,32,32);
        browserArea=showBrowser ? area.removeFromLeft(232) : juce::Rectangle<int>{}; browser.setVisible(showBrowser); if(showBrowser) browser.setBounds(browserArea);
        mixerArea=showMixer ? area.removeFromBottom(std::clamp(area.getHeight()/3,184,224)) : juce::Rectangle<int>{}; arrangementArea=area;
        auto top=area.removeFromTop(44); addTrack.setBounds(top.getX()+14,top.getY()+8,87,29); addClip.setBounds(top.getX()+107,top.getY()+8,93,29);
        duplicate.setBounds(top.getX()+212,top.getY()+8,30,29); remove.setBounds(top.getX()+248,top.getY()+8,30,29);
        zoomOut.setBounds(top.getRight()-75,top.getY()+8,27,29); zoomIn.setBounds(top.getRight()-42,top.getY()+8,27,29);
        rulerArea=area.removeFromTop(30); headers.setBounds(area.removeFromLeft(192)); lanes.setBounds(area);
        canvas.beatWidth=std::max(8.0,(lanes.getWidth()-10)/Workspace::duration)*zoom;
        canvas.setSize(static_cast<int>(std::ceil(Workspace::duration*canvas.beatWidth))+4,std::max(lanes.getHeight()-10,static_cast<int>(model.tracks().size())*rowHeight));
        headers.scroll=lanes.getViewPositionY(); headers.resized();
        mixerView.setVisible(showMixer);
        if(showMixer) { mixerView.setBounds(mixerArea.withTrimmedTop(30).withTrimmedRight(130)); mixerContent.setSize(std::max(mixerView.getWidth(),channels.size()*112),mixerView.getHeight()-7); for(int n=0;n<channels.size();++n) channels[n]->setBounds(n*112,0,112,mixerContent.getHeight()); }
        preview.setBounds(owner.getWidth()-102,statusArea.getY(),91,23); helpPanel.setBounds(owner.getLocalBounds()); helpPanel.setVisible(helpOpen);
    }
    void paint(juce::Graphics& g) {
        g.fillAll(bg); g.setColour(panel); g.fillRect(toolbarArea); g.setColour(line); g.drawHorizontalLine(toolbarArea.getBottom()-1,0,static_cast<float>(owner.getWidth()));
        g.drawVerticalLine(280,18,44); g.drawVerticalLine(603,18,44);
        ui::label(g,"bpm",{666,17,32,29},12,muted); ui::label(g,"4/4",{709,17,40,29},13,muted);
        if(showMixer) {
            g.setColour(panel); g.fillRect(mixerArea); g.setColour(line); g.drawHorizontalLine(mixerArea.getY(),static_cast<float>(mixerArea.getX()),static_cast<float>(mixerArea.getRight()));
            ui::label(g,"Mixer",{mixerArea.getX()+16,mixerArea.getY(),120,29},12,muted);
            const int x=mixerArea.getRight()-113, y=mixerArea.getY()+40;
            ui::glyph(g,ui::Glyph::speaker,{static_cast<float>(x),static_cast<float>(y+2),17,17},muted); ui::label(g,"Master",{x+25,y,80,23},13);
            g.setColour(bg); g.fillRoundedRectangle(static_cast<float>(x+15),static_cast<float>(y+40),5,static_cast<float>(std::max(32,mixerArea.getHeight()-110)),2); g.fillRoundedRectangle(static_cast<float>(x+28),static_cast<float>(y+40),5,static_cast<float>(std::max(32,mixerArea.getHeight()-110)),2);
            ui::label(g,"-inf",{x+50,y+40,50,24},13,muted);
        }
        g.setColour(panel); g.fillRect(rulerArea); ui::label(g,"Tracks",rulerArea.withWidth(192).reduced(16,0),12,muted);
        const int origin=lanes.getX()-lanes.getViewPositionX();
        { juce::Graphics::ScopedSaveState saved(g); g.reduceClipRegion(rulerArea.withTrimmedLeft(192));
          for(int bar=0;bar<16;++bar) { const int x=origin+static_cast<int>(bar*4*canvas.beatWidth); ui::label(g,juce::String(bar+1),{x+6,rulerArea.getY(),45,30},12,bar%4==0 ? ink:muted); }
          const int px=origin+static_cast<int>(model.position()*canvas.beatWidth); g.setColour(ink); g.fillRoundedRectangle(static_cast<float>(px),static_cast<float>(rulerArea.getY()+22),5,7,1);
        }
        g.setColour(panel); g.fillRect(statusArea); ui::label(g,juce::String(static_cast<int>(model.tracks().size()))+" tracks",statusArea.reduced(15,0).withWidth(160),11,muted);
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
        case ShellAction::addClip:i.model.addClip();break;
        case ShellAction::remove:if(i.model.selectedClip()!=0)i.model.removeClip();else i.model.removeTrack();break;
        case ShellAction::duplicate:i.model.duplicateClip();break;
        case ShellAction::undo:i.model.undo();break;
        case ShellAction::redo:i.model.redo();break;
        case ShellAction::browser:i.showBrowser=!i.showBrowser;break;
        case ShellAction::mixer:i.showMixer=!i.showMixer;break;
        case ShellAction::help:i.helpOpen=!i.helpOpen;break;
    }
    i.refresh();
    if(action==ShellAction::help) { if(i.helpOpen)i.helpPanel.close.grabKeyboardFocus();else grabKeyboardFocus(); }
}
bool WorkspaceView::keyPressed(const juce::KeyPress& key) {
    if(dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent())!=nullptr)return false;
    if(impl_->helpOpen) { if(key.getKeyCode()==juce::KeyPress::escapeKey)perform(ShellAction::help);return true; }
    if(key.getModifiers().isCtrlDown()) {
        const auto c=key.getKeyCode();
        if(c=='N')perform(ShellAction::newSession);else if(c=='Z')perform(ShellAction::undo);else if(c=='Y')perform(ShellAction::redo);else if(c=='D')perform(ShellAction::duplicate);else if(c=='S')perform(ShellAction::help);else return false;
        return true;
    }
    if(key.getKeyCode()==juce::KeyPress::spaceKey){perform(ShellAction::playPause);return true;}
    if(key.getKeyCode()==juce::KeyPress::returnKey){perform(ShellAction::stop);return true;}
    if(key.getKeyCode()==juce::KeyPress::deleteKey){perform(ShellAction::remove);return true;}
    switch(key.getKeyCode()) {
        case 'B':perform(ShellAction::browser);break;
        case 'M':perform(ShellAction::mixer);break;
        case '?':perform(ShellAction::help);break;
        default:return false;
    }
    return true;
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
    if(!i.model.tracks().empty() || i.browser.resultCount()!=0 || i.addClip.isEnabled() || i.remove.isEnabled())return "Empty session controls failed";
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
    juce::Label* levelText=nullptr;
    for(int n=0;n<i.channels[0]->level.getNumChildComponents();++n)
        if(auto* candidate=dynamic_cast<juce::Label*>(i.channels[0]->level.getChildComponent(n)))levelText=candidate;
    if(levelText==nullptr)return "Editable mixer level field missing";
    levelText->showEditor();
    if(levelText->getCurrentTextEditor()==nullptr)return "Mixer level text editor missing";
    levelText->getCurrentTextEditor()->setText("-9.5 dB",false); levelText->hideEditor(false);
    if(std::abs(i.model.tracks()[0].level+9.5)>0.001)return "Mixer dB text entry failed";
    for(const double value:{-60.0,6.0}) { i.channels[0]->level.setValue(value,juce::sendNotificationSync); if(i.model.tracks()[0].level!=value)return "Mixer level boundary failed"; }
    i.channels[0]->level.setValue(-12,juce::sendNotificationSync);
    i.channels[0]->mute.onClick(); i.channels[0]->solo.onClick();
    if(i.model.tracks()[0].muted || i.model.tracks()[0].solo)return "Mixer mute/solo failed";
    i.snap.setSelectedId(1,juce::sendNotificationSync); if(i.model.grid!=4)return "Snap control failed";
    i.browser.setQuery("nonexistent"); if(i.browser.resultCount()!=0 || i.browser.activateResult(0))return "No-result browser failed";
    i.browser.setQuery(" rEnAm "); if(i.browser.resultCount()!=1 || !clickIn(i.browser,"browser-row-0"))return "Session search/selection failed";
    i.browser.search.grabKeyboardFocus();
    if(keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)) || i.model.playing() || keyPressed(juce::KeyPress('B')))return "Search shortcut isolation failed";
    grabKeyboardFocus();
    if(!clickIn(i.browser,"browser-templates") || i.browser.resultCount()!=4)return "Template tab failed";
    i.model.seek(12); i.browser.setQuery("rhythm");
    if(i.model.position()!=12 || i.browser.resultCount()!=1 || !clickIn(i.browser,"browser-row-0") || i.model.tracks().size()!=2 || i.model.tracks()[1].clips.size()!=4)return "Rhythm template failed";
    if(!clickIn(i.browser,"browser-session"))return "Session tab failed";
    i.browser.setQuery("rhythm");
    if(i.browser.resultCount()!=1 || !clickIn(i.browser,"browser-row-0") || i.model.selectedTrack()!=1)return "Session navigation failed";
    i.headers.names[1]->setText("Navigator",juce::sendNotificationSync);
    if(i.browser.resultCount()!=0)return "Browser rename/filter synchronization failed";
    i.browser.setQuery({}); if(i.browser.resultCount()!=2)return "Browser clear filter failed";
    if(i.browser.activateResult(-1) || i.browser.activateResult(2))return "Invalid browser row accepted";
    if(keyPressed(juce::KeyPress('I')) || findChildWithID("inspector")!=nullptr)return "Removed inspector is still reachable";
    if(!keyPressed(juce::KeyPress('B')) || i.showBrowser || !keyPressed(juce::KeyPress('B')) || !i.showBrowser)return "Browser toggle failed";
    perform(ShellAction::mixer); if(i.mixerView.isVisible())return "Mixer hide failed"; perform(ShellAction::mixer);
    perform(ShellAction::playPause); if(!i.model.playing())return "Visual play failed"; perform(ShellAction::stop);
    perform(ShellAction::undo); perform(ShellAction::redo); perform(ShellAction::help);
    if(!i.helpPanel.isVisible())return "Help failed";
    if(!keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)) || i.helpPanel.isVisible())return "Help Escape failed";
    for(const auto size:{juce::Point<int>{1080,640},juce::Point<int>{1080,700},juce::Point<int>{1440,900},juce::Point<int>{1920,1080}}) {
        setSize(size.x,size.y);
        if(i.toolbarArea.getHeight()!=62 || i.lanes.getWidth()<640 || i.lanes.getHeight()<200 || i.lanes.getRight()!=getWidth() || i.addTrack.getBounds().intersects(i.zoomOut.getBounds()) || i.snap.getRight()>i.browserToggle.getX())return "Responsive reclaimed-space layout failed";
        if(!createComponentSnapshot(getLocalBounds()).isValid())return "Component render failed";
    }
    // Exercise all starter forms, the 24-track cap, disabled rows, browser scrolling and undo.
    perform(ShellAction::newSession); clickIn(i.browser,"browser-templates");
    for(int n=0;n<Workspace::maxTracks;++n) {
        if(!clickIn(i.browser,"browser-row-"+juce::String(n%4)))return "Template creation failed";
        const auto count=i.model.tracks().back().clips.size();
        const std::size_t expected[]{0,1,4,2}; if(count!=expected[n%4])return "Template sketch contents failed";
    }
    if(i.addTrack.isEnabled() || clickIn(i.browser,"browser-row-0") || i.browser.activateResult(0))return "Track limit not respected";
    clickIn(i.browser,"browser-session"); if(i.browser.resultCount()!=Workspace::maxTracks || !clickIn(i.browser,"browser-row-23") || i.model.selectedTrack()!=23)return "Scrollable session browser failed";
    bool browserScrolled=false;
    for(int n=0;n<i.browser.getNumChildComponents();++n)if(auto* viewport=dynamic_cast<juce::Viewport*>(i.browser.getChildComponent(n))) {
        viewport->setViewPosition(0,23*42); browserScrolled=viewport->getViewPositionY()>0;
    }
    if(!browserScrolled)return "Browser viewport did not scroll";
    perform(ShellAction::remove); if(i.browser.resultCount()!=23)return "Browser deletion synchronization failed";
    perform(ShellAction::undo); if(i.browser.resultCount()!=24)return "Browser undo synchronization failed";
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
    perform(ShellAction::demo); i.browser.setTemplateTab(false); i.snap.setSelectedId(3,juce::sendNotificationSync); i.zoom=1; setSize(1440,900); i.refresh();
    return {};
}
} // namespace simplicity
