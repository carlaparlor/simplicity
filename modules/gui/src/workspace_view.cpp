#include <simplicity/workspace_view.h>
#include <algorithm>
#include <cmath>
#include <functional>

namespace simplicity {
namespace {
const juce::Colour bg{0xff111315}, panel{0xff191c1f}, raised{0xff22262a}, line{0xff30353a};
const juce::Colour ink{0xffe6e9e7}, muted{0xff9aa39f}, accent{0xffc1ee85};
const juce::Colour trackColours[]{juce::Colour{0xffa4c98e}, juce::Colour{0xffcba57e}, juce::Colour{0xff8daabf}, juce::Colour{0xffb1a0c9}};
constexpr int rowHeight = 84;
void text(juce::Graphics& g, const juce::String& s, juce::Rectangle<int> r, float size = 13, juce::Colour c = ink, bool bold = false) {
    g.setColour(c); g.setFont(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
    g.drawText(s, r, juce::Justification::centredLeft, true);
}
juce::String number(int n) { return juce::String(n).paddedLeft('0', 2); }
class WindowButton final : public juce::Button {
public:
    int kind;
    explicit WindowButton(int type) : Button(type == juce::DocumentWindow::closeButton ? "Close" : type == juce::DocumentWindow::minimiseButton ? "Minimize" : "Maximize or restore"), kind(type) { setTooltip(getName()); }
    void paintButton(juce::Graphics& g, bool over, bool down) override {
        g.fillAll(over ? (kind == juce::DocumentWindow::closeButton ? juce::Colour{0xffa54141} : line) : (down ? raised : bg));
        g.setColour(ink); const float x = getWidth() * 0.5F, y = getHeight() * 0.5F;
        if (kind == juce::DocumentWindow::closeButton) { g.drawLine(x - 4, y - 4, x + 4, y + 4); g.drawLine(x - 4, y + 4, x + 4, y - 4); }
        else if (kind == juce::DocumentWindow::minimiseButton) g.drawLine(x - 4, y + 3, x + 4, y + 3);
        else g.drawRect(juce::Rectangle<float>{x - 4, y - 4, 8, 8});
    }
};
class BlockLook final : public juce::LookAndFeel_V4 {
public:
    BlockLook() {
        setColour(juce::TextEditor::backgroundColourId, bg); setColour(juce::TextEditor::textColourId, ink);
        setColour(juce::TextEditor::outlineColourId, line); setColour(juce::TextEditor::focusedOutlineColourId, accent);
        setColour(juce::TextEditor::highlightColourId, accent.withAlpha(0.3F));
        setColour(juce::ComboBox::backgroundColourId, raised); setColour(juce::ComboBox::textColourId, ink);
        setColour(juce::ComboBox::outlineColourId, line); setColour(juce::PopupMenu::backgroundColourId, panel);
        setColour(juce::PopupMenu::textColourId, ink); setColour(juce::PopupMenu::highlightedBackgroundColourId, raised);
        setColour(juce::PopupMenu::highlightedTextColourId, accent); setColour(juce::Label::textColourId, ink);
        setColour(juce::ScrollBar::thumbColourId, line.brighter(0.25F));
        setColour(juce::ResizableWindow::backgroundColourId, bg);
    }
    juce::Button* createDocumentWindowButton(int type) override { return new WindowButton(type); }
    void drawDocumentWindowTitleBar(juce::DocumentWindow&, juce::Graphics& g, int width, int height,
                                   int titleX, int titleWidth, const juce::Image*, bool) override {
        g.fillAll(bg); text(g, "simplicity  /  workspace preview", {titleX + 6, 0, std::min(titleWidth, width - titleX), height}, 10, muted);
    }
    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override {
        auto area = b.getLocalBounds(); const bool on = b.getToggleState();
        g.setColour(on ? accent : (down ? line : (over ? raised.brighter(0.15F) : raised)));
        g.fillRect(area); g.setColour(on ? accent : line); g.drawRect(area);
        if (b.hasKeyboardFocus(false)) { g.setColour(accent); g.drawRect(area.reduced(2), 2); }
    }
    void drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) override {
        const auto colour = b.getToggleState() ? bg : (b.isEnabled() ? ink : muted.withAlpha(0.38F));
        g.setColour(colour); g.setFont(juce::FontOptions(12.0F, juce::Font::bold));
        g.drawText(b.getButtonText(), b.getLocalBounds().reduced(6, 0), juce::Justification::centred, true);
    }
    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                          juce::Slider::SliderStyle style, juce::Slider&) override {
        if (style == juce::Slider::LinearVertical) {
            const auto cx = x + w / 2; g.setColour(bg); g.fillRect(cx - 2, y, 4, h);
            g.setColour(accent.withAlpha(0.5F)); g.fillRect(static_cast<float>(cx - 2), pos, 4.0F, static_cast<float>(y + h) - pos);
            g.setColour(ink); g.fillRect(static_cast<float>(cx - 12), pos - 4, 24.0F, 8.0F);
        } else {
            const auto cy = y + h / 2; g.setColour(bg); g.fillRect(x, cy - 2, w, 4);
            g.setColour(accent); g.fillRect(x, cy - 2, std::max(0, static_cast<int>(pos) - x), 4);
            g.setColour(ink); g.fillRect(pos - 4, static_cast<float>(cy - 8), 8.0F, 16.0F);
        }
    }
};
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
            if ((bar / 4) % 2 == 0) { g.setColour(juce::Colour{0xff15181a}); g.fillRect(x, 0, static_cast<int>(4 * beatWidth), getHeight()); }
            g.setColour(line.withAlpha(0.7F)); g.drawVerticalLine(x, 0, static_cast<float>(getHeight()));
            for (int beat = 1; beat < 4; ++beat) { g.setColour(line.withAlpha(0.25F)); g.drawVerticalLine(x + static_cast<int>(beat * beatWidth), 0, static_cast<float>(getHeight())); }
        }
        const auto& tracks = model.tracks();
        for (std::size_t row = 0; row < tracks.size(); ++row) {
            const auto& track = tracks[row]; const auto colour = trackColours[track.colour % 4];
            const int y = static_cast<int>(row) * rowHeight;
            g.setColour(line); g.drawHorizontalLine(y + rowHeight - 1, 0, static_cast<float>(getWidth()));
            for (const auto& c : track.clips) {
                const auto r = clipBounds(c, static_cast<int>(row));
                g.setColour(colour.withAlpha(track.muted ? 0.12F : 0.27F)); g.fillRect(r);
                g.setColour(colour.withAlpha(track.muted ? 0.3F : 0.85F)); g.fillRect(r.getX(), r.getY(), r.getWidth(), 3.0F);
                if (model.selectedClip() == c.id) { g.setColour(ink); g.drawRect(r, 1.5F); }
                text(g, juce::String(c.name), r.toNearestInt().reduced(8, 4).withHeight(21), 11, colour, true);
                // Deliberately illustrative note blocks, not audio waveforms or recorded data.
                g.setColour(colour.withAlpha(track.muted ? 0.15F : 0.65F));
                for (int step = 0; step < static_cast<int>(c.length); ++step) {
                    const float px = r.getX() + 8 + static_cast<float>(step * beatWidth);
                    if (px + 5 < r.getRight() - 6) g.fillRect(px, r.getY() + 33 + static_cast<float>((step * 7 + static_cast<int>(row) * 3) % 3) * 5, static_cast<float>(std::max(3.0, beatWidth * 0.6)), 3.0F);
                }
            }
        }
        if (tracks.empty()) {
            text(g, "Start with one track.", {28, 36, getWidth() - 56, 30}, 23, ink, true);
            text(g, "Use + Track, then + Sketch. Everything can be undone.", {28, 72, getWidth() - 56, 24}, 13, muted);
        }
        const auto x = static_cast<float>(model.position() * beatWidth);
        g.setColour(accent); g.fillRect(x, 0.0F, 1.5F, static_cast<float>(getHeight()));
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
class TrackHeaders final : public juce::Component {
public:
    Workspace& model; std::function<void()> changed; int scroll{};
    juce::OwnedArray<juce::TextButton> names, mute, solo;
    explicit TrackHeaders(Workspace& m) : model(m) {}
    void refresh() {
        if (names.size() != static_cast<int>(model.tracks().size())) {
            names.clear(); mute.clear(); solo.clear();
            for (int i = 0; i < static_cast<int>(model.tracks().size()); ++i) {
                auto* n = names.add(new juce::TextButton()); auto* m = mute.add(new juce::TextButton("M")); auto* s = solo.add(new juce::TextButton("S"));
                addAndMakeVisible(n); addAndMakeVisible(m); addAndMakeVisible(s);
                n->onClick = [this, i] { model.selectTrack(i); changed(); };
                m->onClick = [this, i] { model.selectTrack(i); model.toggleMute(); changed(); };
                s->onClick = [this, i] { model.selectTrack(i); model.toggleSolo(); changed(); };
                m->setTooltip("Mute track (sketch state only)"); s->setTooltip("Solo track (sketch state only)");
            }
        }
        for (int i = 0; i < names.size(); ++i) {
            const auto& t = model.tracks()[static_cast<std::size_t>(i)]; names[i]->setButtonText(t.name); names[i]->setToggleState(model.selectedTrack() == i, juce::dontSendNotification);
            mute[i]->setToggleState(t.muted, juce::dontSendNotification); solo[i]->setToggleState(t.solo, juce::dontSendNotification);
            names[i]->setTooltip("Select " + juce::String(t.name)); mute[i]->setTitle("Mute " + juce::String(t.name)); solo[i]->setTitle("Solo " + juce::String(t.name));
        }
        resized(); repaint();
    }
    void resized() override { for (int i = 0; i < names.size(); ++i) { const int y = i * rowHeight - scroll; names[i]->setBounds(25, y + 10, getWidth() - 37, 27); mute[i]->setBounds(25, y + 44, 28, 25); solo[i]->setBounds(59, y + 44, 28, 25); } }
    void paint(juce::Graphics& g) override {
        g.fillAll(panel);
        for (int i = 0; i < names.size(); ++i) { int y = i * rowHeight - scroll; g.setColour(trackColours[model.tracks()[static_cast<std::size_t>(i)].colour % 4]); g.fillRect(0, y + 10, 3, rowHeight - 20); text(g, number(i + 1), {7, y + 42, 18, 24}, 10, muted); g.setColour(line); g.drawHorizontalLine(y + rowHeight - 1, 0, static_cast<float>(getWidth())); }
    }
};
class Channel final : public juce::Component {
public:
    Workspace& model; int index; std::function<void()> changed;
    juce::TextButton select, mute{"M"}, solo{"S"}; juce::Slider level;
    Channel(Workspace& m, int i) : model(m), index(i) {
        for (auto* c : std::initializer_list<juce::Component*>{&select, &mute, &solo, &level}) addAndMakeVisible(c);
        level.setSliderStyle(juce::Slider::LinearVertical); level.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); level.setRange(-60, 6, 0.1); level.setDoubleClickReturnValue(true, -6);
        level.setName("Track level, dB; visual sketch only"); level.setTooltip("Sketch level only. No audio is connected.");
        level.onDragEnd = [this] { model.selectTrack(index); model.setLevel(level.getValue()); changed(); };
        // Keyboard changes have no drag-end notification.
        level.onValueChange = [this] { if (!level.isMouseButtonDown()) { model.selectTrack(index); model.setLevel(level.getValue()); if (changed) changed(); } repaint(); };
        select.onClick = [this] { model.selectTrack(index); changed(); };
        mute.onClick = [this] { model.selectTrack(index); model.toggleMute(); changed(); };
        solo.onClick = [this] { model.selectTrack(index); model.toggleSolo(); changed(); };
        mute.setTooltip("Mute track (sketch state only)"); solo.setTooltip("Solo track (sketch state only)");
    }
    void refresh() { const auto& t = model.tracks()[static_cast<std::size_t>(index)]; select.setButtonText(t.name); select.setToggleState(model.selectedTrack() == index, juce::dontSendNotification); mute.setToggleState(t.muted, juce::dontSendNotification); solo.setToggleState(t.solo, juce::dontSendNotification); if (!level.isMouseButtonDown()) level.setValue(t.level, juce::dontSendNotification); repaint(); }
    void resized() override { select.setBounds(8, 8, getWidth() - 16, 25); mute.setBounds(12, 40, 27, 23); solo.setBounds(45, 40, 27, 23); level.setBounds(17, 70, 50, getHeight() - 99); }
    void paint(juce::Graphics& g) override {
        g.fillAll(panel); g.setColour(line); g.drawVerticalLine(getWidth() - 1, 0, static_cast<float>(getHeight()));
        g.setColour(trackColours[model.tracks()[static_cast<std::size_t>(index)].colour % 4]); g.fillRect(8, 0, getWidth() - 16, 2);
        text(g, juce::String(level.getValue(), 1) + " dB", {12, getHeight() - 25, getWidth() - 16, 20}, 11, muted);
        g.setColour(bg); g.fillRect(getWidth() - 26, 75, 7, std::max(1, getHeight() - 110));
    }
};
class HelpPanel final : public juce::Component {
public:
    juce::TextButton close{"Got it"};
    HelpPanel() { addAndMakeVisible(close); setAlwaysOnTop(true); }
    void resized() override { close.setBounds(getWidth() / 2 - 60, getHeight() / 2 + 134, 120, 34); }
    void paint(juce::Graphics& g) override {
        g.fillAll(bg.withAlpha(0.94F)); auto r = juce::Rectangle<int>(getWidth() / 2 - 250, getHeight() / 2 - 210, 500, 400);
        g.setColour(panel); g.fillRect(r); g.setColour(line); g.drawRect(r);
        r = r.reduced(30); text(g, "Less searching. More making.", r.removeFromTop(38), 24, ink, true);
        text(g, "The essentials are one key away.", r.removeFromTop(36), 13, muted);
        for (const auto& s : {"Space     Play / pause the visual transport", "Enter      Stop and return to the start", "Ctrl N     New sketch       Ctrl Z / Y     Undo / redo", "Ctrl D     Duplicate clip    Delete     Remove selection", "B / M / I   Browser / mixer / inspector", "+ Track   Add a lane; double-click an empty lane to sketch."}) text(g, s, r.removeFromTop(29), 13);
        text(g, "GUI preview only. No sound, recording or project saving yet.", r.removeFromTop(40), 12, accent);
    }
};
} // namespace

class WorkspaceView::Impl final : private juce::Timer {
public:
    WorkspaceView& owner; Workspace model; BlockLook look;
    juce::TooltipWindow tooltips{&owner, 550};
    juce::Label title, position;
    juce::TextEditor search, tempo, trackName;
    juce::TextButton play{"Play"}, stop{"Stop"}, record{"Record"}, undo{"Undo"}, redo{"Redo"}, fresh{"New"}, demo{"Demo"}, help{"?"};
    juce::TextButton browserToggle{"Browser"}, mixerToggle{"Mixer"}, inspectorToggle{"Inspector"}, loop{"Loop"}, addTrack{"+ Track"}, addClip{"+ Sketch"}, zoomOut{"-"}, zoomIn{"+"};
    juce::TextButton starter{"Empty track"}, patterned{"Track + sketch"}, mute{"Mute"}, solo{"Solo"}, removeTrack{"Delete track"}, duplicate{"Duplicate"}, removeClip{"Delete clip"};
    juce::ComboBox snap;
    juce::Slider level;
    Canvas canvas{model}; ScrollingView lanes; TrackHeaders headers{model};
    juce::Component mixerContent; juce::Viewport mixerView;
    juce::OwnedArray<Channel> channels;
    HelpPanel helpPanel;
    bool showBrowser{true}, showMixer{true}, showInspector{true}, helpOpen{};
    double zoom{1}, previousTime{juce::Time::getMillisecondCounterHiRes()};
    juce::Rectangle<int> headerArea, transportArea, browserArea, inspectorArea, arrangementArea, rulerArea, mixerArea, statusArea;
    juce::String status{"Demo layout. Sketch clips are illustrative; no audio is connected."};
    explicit Impl(WorkspaceView& o) : owner(o) {
        owner.setLookAndFeel(&look); owner.setWantsKeyboardFocus(true); owner.setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);
        owner.setName("Simplicity workspace");
        for (auto* c : std::initializer_list<juce::Component*>{&title, &position, &search, &tempo, &trackName, &play, &stop, &record, &undo, &redo, &fresh, &demo, &help, &browserToggle, &mixerToggle, &inspectorToggle, &loop, &addTrack, &addClip, &zoomOut, &zoomIn, &starter, &patterned, &mute, &solo, &removeTrack, &duplicate, &removeClip, &snap, &level, &lanes, &headers, &mixerView}) owner.addAndMakeVisible(c);
        owner.addChildComponent(helpPanel);
        title.setFont(juce::FontOptions(17, juce::Font::bold)); title.setEditable(false, true); title.setTooltip("Double-click to rename this unsaved sketch.");
        title.onTextChange = [this] { model.setTitle(title.getText().toStdString()); refresh(); };
        position.setFont(juce::FontOptions(25, juce::Font::plain));
        search.setTextToShowWhenEmpty("Search track starters...", muted); search.setName("Search track starters"); search.onTextChange = [this] { filterStarters(); };
        tempo.setName("Tempo in beats per minute, 40 to 240"); tempo.setInputRestrictions(3, "0123456789");
        tempo.onReturnKey = [this] { model.setBpm(tempo.getText().getDoubleValue()); refresh(); owner.grabKeyboardFocus(); };
        tempo.onFocusLost = [this] { model.setBpm(tempo.getText().getDoubleValue()); refresh(); };
        trackName.setName("Selected track name"); trackName.setInputRestrictions(48);
        trackName.onReturnKey = [this] { model.renameTrack(trackName.getText().toStdString()); refresh(); owner.grabKeyboardFocus(); };
        trackName.onFocusLost = [this] { model.renameTrack(trackName.getText().toStdString()); refresh(); };
        bind(play, ShellAction::playPause, "Play / pause visual transport (Space). No audio output."); bind(stop, ShellAction::stop, "Return to start (Enter)");
        record.setEnabled(false); record.setTooltip("Recording is not implemented yet.");
        bind(undo, ShellAction::undo, "Undo sketch edit (Ctrl Z)"); bind(redo, ShellAction::redo, "Redo sketch edit (Ctrl Y)");
        bind(fresh, ShellAction::newSession, "New empty sketch (Ctrl N). Undo restores your previous sketch."); bind(demo, ShellAction::demo, "Load illustrative demo layout. No media or instruments."); bind(help, ShellAction::help, "Keyboard shortcuts and preview limitations");
        bind(browserToggle, ShellAction::browser, "Show / hide browser (B)"); bind(mixerToggle, ShellAction::mixer, "Show / hide mixer (M)"); bind(inspectorToggle, ShellAction::inspector, "Show / hide inspector (I)");
        bind(addTrack, ShellAction::addTrack, "Add an empty sketch track"); bind(addClip, ShellAction::addClip, "Add a four-beat sketch clip at the playhead");
        bind(duplicate, ShellAction::duplicate, "Duplicate selected clip (Ctrl D)");
        removeTrack.onClick = [this] { model.removeTrack(); refresh(); }; removeClip.onClick = [this] { model.removeClip(); refresh(); };
        loop.onClick = [this] { model.loop = !model.loop; refresh(); }; loop.setTooltip("Loop the visual 16-bar sketch. No sound.");
        starter.onClick = [this] { owner.perform(ShellAction::addTrack); };
        patterned.onClick = [this] { model.addTrack("Sketch " + std::to_string(model.tracks().size() + 1)); model.addClip(); refresh(); };
        starter.setTooltip("Add a track with no clips"); patterned.setTooltip("Add a track and an illustrative four-beat clip");
        mute.onClick = [this] { model.toggleMute(); refresh(); }; solo.onClick = [this] { model.toggleSolo(); refresh(); };
        mute.setTooltip("Mute sketch state only"); solo.setTooltip("Solo sketch state only");
        snap.addItem("Bar", 1); snap.addItem("1/2 beat", 2); snap.addItem("1/4 beat", 3); snap.addItem("Off", 4); snap.setSelectedId(3, juce::dontSendNotification); model.grid = 0.25;
        snap.setName("Clip movement snap grid"); snap.onChange = [this] { const double values[]{4, 0.5, 0.25, 0}; model.grid = values[std::clamp(snap.getSelectedId() - 1, 0, 3)]; };
        zoomOut.onClick = [this] { zoom = std::max(0.75, zoom - 0.25); owner.resized(); };
        zoomIn.onClick = [this] { zoom = std::min(3.0, zoom + 0.25); owner.resized(); };
        zoomOut.setTooltip("Zoom out arrangement"); zoomIn.setTooltip("Zoom in arrangement");
        level.setSliderStyle(juce::Slider::LinearHorizontal); level.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 22); level.setRange(-60, 6, 0.1); level.setTextValueSuffix(" dB"); level.setName("Selected track level; sketch only");
        level.onDragEnd = [this] { model.setLevel(level.getValue()); refresh(); };
        level.onValueChange = [this] { if (!level.isMouseButtonDown()) { model.setLevel(level.getValue()); refresh(); } };
        lanes.setViewedComponent(&canvas, false); lanes.setScrollBarsShown(true, true); lanes.setScrollBarThickness(8);
        lanes.scrolled = [this] { headers.scroll = lanes.getViewPositionY(); headers.resized(); headers.repaint(); owner.repaint(); };
        canvas.changed = [this] { refresh(); }; headers.changed = [this] { refresh(); };
        mixerView.setViewedComponent(&mixerContent, false); mixerView.setScrollBarsShown(false, true); mixerView.setScrollBarThickness(8);
        helpPanel.close.onClick = [this] { owner.perform(ShellAction::help); };
        setIDs(); startTimerHz(30);
    }
    ~Impl() override { stopTimer(); owner.setLookAndFeel(nullptr); }
    void setIDs() {
        play.setComponentID("play"); stop.setComponentID("stop"); fresh.setComponentID("new"); demo.setComponentID("demo"); addTrack.setComponentID("add-track"); addClip.setComponentID("add-clip");
        search.setComponentID("search"); tempo.setComponentID("tempo"); trackName.setComponentID("track-name"); level.setComponentID("level"); snap.setComponentID("snap");
        mute.setComponentID("mute"); solo.setComponentID("solo"); removeTrack.setComponentID("remove-track"); duplicate.setComponentID("duplicate"); removeClip.setComponentID("remove-clip");
        browserToggle.setComponentID("browser"); mixerToggle.setComponentID("mixer"); inspectorToggle.setComponentID("inspector");
    }
    void bind(juce::TextButton& button, ShellAction action, const juce::String& tip) { button.setTooltip(tip); button.onClick = [this, action] { owner.perform(action); }; }
    void filterStarters() { const auto q = search.getText().trim(); starter.setVisible(showBrowser && juce::String("Empty track").containsIgnoreCase(q)); patterned.setVisible(showBrowser && juce::String("Track + sketch").containsIgnoreCase(q)); owner.repaint(); }
    void wireShortcuts(juce::Component& component) {
        component.addKeyListener(&owner);
        for (int n = 0; n < component.getNumChildComponents(); ++n) wireShortcuts(*component.getChildComponent(n));
    }
    void refresh() {
        title.setText(model.title(), juce::dontSendNotification);
        if (!tempo.hasKeyboardFocus(true)) tempo.setText(juce::String(model.bpm(), 0), false);
        const bool selected = model.selectedTrack() >= 0;
        if (selected && !trackName.hasKeyboardFocus(true)) trackName.setText(model.tracks()[static_cast<std::size_t>(model.selectedTrack())].name, false);
        if (!selected) trackName.setText({}, false);
        for (auto* c : std::initializer_list<juce::Component*>{&trackName, &level, &mute, &solo, &removeTrack, &addClip}) c->setEnabled(selected);
        if (selected) { const auto& t = model.tracks()[static_cast<std::size_t>(model.selectedTrack())]; mute.setToggleState(t.muted, juce::dontSendNotification); solo.setToggleState(t.solo, juce::dontSendNotification); if (!level.isMouseButtonDown()) level.setValue(t.level, juce::dontSendNotification); }
        duplicate.setEnabled(model.selectedClip() != 0); removeClip.setEnabled(model.selectedClip() != 0);
        addTrack.setEnabled(model.tracks().size() < Workspace::maxTracks); starter.setEnabled(addTrack.isEnabled()); patterned.setEnabled(addTrack.isEnabled());
        undo.setEnabled(model.canUndo()); redo.setEnabled(model.canRedo()); play.setToggleState(model.playing(), juce::dontSendNotification); play.setButtonText(model.playing() ? "Pause" : "Play"); loop.setToggleState(model.loop, juce::dontSendNotification);
        browserToggle.setToggleState(showBrowser, juce::dontSendNotification); mixerToggle.setToggleState(showMixer, juce::dontSendNotification); inspectorToggle.setToggleState(showInspector, juce::dontSendNotification);
        headers.refresh();
        if (channels.size() != static_cast<int>(model.tracks().size())) { channels.clear(); for (int i = 0; i < static_cast<int>(model.tracks().size()); ++i) { auto* c = channels.add(new Channel(model, i)); c->changed = [this] { refresh(); }; mixerContent.addAndMakeVisible(c); } }
        for (auto* c : channels) c->refresh();
        wireShortcuts(owner); updatePosition(); owner.resized(); owner.repaint(); canvas.repaint();
    }
    void updatePosition() { const double p = model.position(); position.setText(number(static_cast<int>(p) / 4 + 1) + " : " + number(static_cast<int>(p) % 4 + 1) + " : " + number(static_cast<int>((p - std::floor(p)) * 100)), juce::dontSendNotification); }
    void timerCallback() override {
        const auto now = juce::Time::getMillisecondCounterHiRes(); const bool wasPlaying = model.playing();
        model.advance((now - previousTime) / 1000.0); previousTime = now;
        if (wasPlaying) { updatePosition(); canvas.repaint(); owner.repaint(rulerArea); if (!model.playing()) refresh(); }
    }
    void layout() {
        auto area = owner.getLocalBounds(); headerArea = area.removeFromTop(64); transportArea = area.removeFromTop(66); statusArea = area.removeFromBottom(28);
        title.setBounds(235, 10, std::max(170, owner.getWidth() - 850), 26);
        const int right = owner.getWidth(); fresh.setBounds(right - 360, 17, 50, 30); demo.setBounds(right - 304, 17, 58, 30); undo.setBounds(right - 232, 17, 58, 30); redo.setBounds(right - 168, 17, 58, 30); help.setBounds(right - 52, 17, 30, 30);
        play.setBounds(22, 80, 62, 32); stop.setBounds(90, 80, 54, 32); record.setBounds(150, 80, 62, 32); position.setBounds(235, 78, 186, 35);
        tempo.setBounds(454, 80, 53, 32); loop.setBounds(553, 80, 52, 32); snap.setBounds(664, 80, 91, 32);
        browserToggle.setBounds(right - 303, 80, 84, 32); mixerToggle.setBounds(right - 211, 80, 74, 32); inspectorToggle.setBounds(right - 129, 80, 107, 32);
        browserArea = showBrowser ? area.removeFromLeft(202) : juce::Rectangle<int>{};
        inspectorArea = showInspector ? area.removeFromRight(218) : juce::Rectangle<int>{};
        mixerArea = showMixer ? area.removeFromBottom(std::clamp(area.getHeight() / 3 + 40, 184, 218)) : juce::Rectangle<int>{}; arrangementArea = area;
        auto top = area.removeFromTop(43); addTrack.setBounds(top.getX() + 14, top.getY() + 9, 76, 27); addClip.setBounds(top.getX() + 96, top.getY() + 9, 82, 27);
        zoomOut.setBounds(top.getRight() - 70, top.getY() + 9, 25, 27); zoomIn.setBounds(top.getRight() - 39, top.getY() + 9, 25, 27);
        rulerArea = area.removeFromTop(31); headers.setBounds(area.removeFromLeft(180)); lanes.setBounds(area);
        canvas.beatWidth = std::max(8.0, (lanes.getWidth() - 10) / Workspace::duration) * zoom;
        canvas.setSize(static_cast<int>(std::ceil(Workspace::duration * canvas.beatWidth)) + 4, std::max(lanes.getHeight() - 10, static_cast<int>(model.tracks().size()) * rowHeight));
        headers.scroll = lanes.getViewPositionY(); headers.resized();
        search.setVisible(showBrowser); starter.setVisible(showBrowser); patterned.setVisible(showBrowser);
        if (showBrowser) { const auto x = browserArea.getX() + 16; search.setBounds(x, browserArea.getY() + 48, 170, 32); starter.setBounds(x, browserArea.getY() + 133, 170, 38); patterned.setBounds(x, browserArea.getY() + 179, 170, 38); } filterStarters();
        for (auto* c : std::initializer_list<juce::Component*>{&trackName, &level, &mute, &solo, &removeTrack, &duplicate, &removeClip}) c->setVisible(showInspector);
        if (showInspector) { const int x = inspectorArea.getX() + 18, y = inspectorArea.getY(); trackName.setBounds(x, y + 78, 182, 33); level.setBounds(x, y + 149, 182, 58); mute.setBounds(x, y + 220, 87, 29); solo.setBounds(x + 95, y + 220, 87, 29); removeTrack.setBounds(x, y + 263, 182, 28); duplicate.setBounds(x, y + 440, 86, 29); removeClip.setBounds(x + 94, y + 440, 88, 29); }
        mixerView.setVisible(showMixer);
        if (showMixer) { mixerView.setBounds(mixerArea.withTrimmedTop(34).withTrimmedRight(135)); mixerContent.setSize(std::max(mixerView.getWidth(), channels.size() * 112), mixerView.getHeight() - 8); for (int i = 0; i < channels.size(); ++i) channels[i]->setBounds(i * 112, 0, 112, mixerContent.getHeight()); }
        helpPanel.setBounds(owner.getLocalBounds()); helpPanel.setVisible(helpOpen);
    }
    void paint(juce::Graphics& g) {
        g.fillAll(bg); g.setColour(panel); g.fillRect(headerArea); g.fillRect(browserArea); g.fillRect(inspectorArea);
        g.setColour(raised.darker(0.15F)); g.fillRect(transportArea); g.setColour(line); g.drawHorizontalLine(63, 0, static_cast<float>(owner.getWidth())); g.drawHorizontalLine(129, 0, static_cast<float>(owner.getWidth()));
        for (int i = 0; i < 4; ++i) { g.setColour(accent); g.fillRect(23 + i * 6, 25 - (i % 3) * 3, 4, 16 + (i % 3) * 3); }
        text(g, "simplicity", {61, 13, 151, 35}, 24, ink, true);
        text(g, "UNSAVED SKETCH  /  GUI PREVIEW", {239, 36, 310, 16}, 9, muted, true);
        text(g, "BPM", {511, 83, 38, 26}, 10, muted); text(g, "SNAP", {624, 82, 38, 27}, 10, muted);
        g.setColour(line); g.drawVerticalLine(222, 78, 114);
        if (showBrowser) {
            const int x = browserArea.getX() + 16, y = browserArea.getY();
            text(g, "BROWSER", {x, y + 12, 170, 28}, 11, muted, true); text(g, "TRACK STARTERS", {x, y + 99, 170, 26}, 10, muted, true);
            if (!starter.isVisible() && !patterned.isVisible()) text(g, "No starters match.", {x, y + 139, 170, 30}, 12, muted);
            text(g, "A place for your next idea.", {x, y + 238, 170, 23}, 12, ink);
            text(g, "Add a track. Add a sketch.", {x, y + 261, 170, 23}, 11, muted);
            text(g, "SHELL PREVIEW", {x, browserArea.getBottom() - 78, 170, 22}, 10, accent, true);
            text(g, "Audio is not connected.", {x, browserArea.getBottom() - 52, 170, 22}, 11, muted);
            text(g, "Nothing is recorded or saved.", {x, browserArea.getBottom() - 33, 170, 20}, 10, muted);
            g.setColour(line); g.drawVerticalLine(browserArea.getRight() - 1, static_cast<float>(y), static_cast<float>(browserArea.getBottom()));
        }
        text(g, "ARRANGEMENT", {arrangementArea.getX() + 196, arrangementArea.getY() + 9, 120, 27}, 10, muted, true);
        text(g, juce::String(static_cast<int>(model.tracks().size())) + " tracks  /  16 bars", {arrangementArea.getX() + 319, arrangementArea.getY() + 9, 170, 27}, 10, muted);
        g.setColour(panel); g.fillRect(rulerArea); text(g, "TRACKS", rulerArea.withWidth(180).reduced(15, 0), 10, muted, true);
        const int origin = lanes.getX() - lanes.getViewPositionX();
        { juce::Graphics::ScopedSaveState save(g); g.reduceClipRegion(rulerArea.withTrimmedLeft(180));
          for (int bar = 0; bar <= 16; ++bar) { const int x = origin + static_cast<int>(bar * 4 * canvas.beatWidth); text(g, juce::String(bar + 1), {x + 6, rulerArea.getY(), 45, 30}, 10, (bar % 4 == 0) ? ink : muted); }
          const int px = origin + static_cast<int>(model.position() * canvas.beatWidth); g.setColour(accent); g.fillRect(px, rulerArea.getY() + 22, 6, 8);
        }
        if (showInspector) {
            const int x = inspectorArea.getX() + 18, y = inspectorArea.getY();
            text(g, "INSPECTOR", {x, y + 12, 182, 28}, 11, muted, true);
            text(g, model.selectedTrack() >= 0 ? "TRACK " + number(model.selectedTrack() + 1) : "NO TRACK SELECTED", {x, y + 49, 182, 23}, 10, accent, true);
            text(g, "LEVEL  /  SKETCH ONLY", {x, y + 125, 182, 23}, 10, muted, true);
            g.setColour(line); g.drawHorizontalLine(y + 315, static_cast<float>(x), static_cast<float>(x + 182));
            text(g, "CLIP", {x, y + 330, 182, 24}, 10, muted, true);
            const auto* c = model.clip(model.selectedClip());
            text(g, c ? juce::String(c->name) : "Select a sketch clip", {x, y + 363, 182, 25}, 13, ink, true);
            text(g, c ? "Start  " + juce::String(c->beat / 4 + 1, 2) + "  /  " + juce::String(c->length / 4, 1) + " bars" : "Click a block in the arrangement.", {x, y + 395, 182, 24}, 10, muted);
            if (inspectorArea.getHeight() > 580) {
                text(g, "Move clips by dragging.", {x, inspectorArea.getBottom() - 62, 182, 22}, 11, muted);
                text(g, "Ctrl Z always has your back.", {x, inspectorArea.getBottom() - 40, 182, 22}, 11, muted);
            }
            g.setColour(line); g.drawVerticalLine(inspectorArea.getX(), static_cast<float>(y), static_cast<float>(inspectorArea.getBottom()));
        }
        if (showMixer) {
            g.setColour(panel); g.fillRect(mixerArea); text(g, "MIXER", {mixerArea.getX() + 15, mixerArea.getY(), 95, 32}, 10, muted, true);
            text(g, "LEVELS ARE SKETCH STATE  /  NO SIGNAL", {mixerArea.getX() + 110, mixerArea.getY(), 300, 32}, 9, muted);
            const int x = mixerArea.getRight() - 119, y = mixerArea.getY() + 43;
            text(g, "MASTER", {x, y, 104, 22}, 11, ink, true); text(g, "No audio", {x, y + 26, 104, 22}, 11, muted);
            g.setColour(bg); g.fillRect(x, y + 62, 88, 42); text(g, "-inf", {x + 12, y + 70, 65, 22}, 17, muted);
            text(g, "OUTPUT OFFLINE", {x, y + 119, 104, 22}, 9, muted);
            g.setColour(line); g.drawHorizontalLine(mixerArea.getY(), static_cast<float>(mixerArea.getX()), static_cast<float>(mixerArea.getRight()));
        }
        g.setColour(raised); g.fillRect(statusArea); text(g, status, statusArea.reduced(16, 0).withTrimmedRight(242), 10, muted);
        text(g, "GUI PREVIEW   /   NO AUDIO OUTPUT", {statusArea.getRight() - 227, statusArea.getY(), 218, 28}, 9, accent, true);
    }
};

WorkspaceView::WorkspaceView() { impl_ = std::make_unique<Impl>(*this); setSize(1440, 900); impl_->refresh(); }
WorkspaceView::~WorkspaceView() = default;
void WorkspaceView::paint(juce::Graphics& g) { impl_->paint(g); }
void WorkspaceView::resized() { if (impl_) impl_->layout(); }
const Workspace& WorkspaceView::workspace() const noexcept { return impl_->model; }
void WorkspaceView::perform(ShellAction action) {
    auto& i = *impl_;
    switch (action) {
        case ShellAction::playPause: i.model.togglePlay(); break;
        case ShellAction::stop: i.model.stop(); break;
        case ShellAction::newSession: i.model.newSession(); i.status = "Empty sketch. Add a track to get started. Ctrl Z restores the previous layout."; break;
        case ShellAction::demo: i.model.resetDemo(); i.status = "Illustrative demo layout. These blocks contain no audio or MIDI data."; break;
        case ShellAction::addTrack: if (!i.model.addTrack("Track " + std::to_string(i.model.tracks().size() + 1))) i.status = "This shell supports up to 24 sketch tracks."; break;
        case ShellAction::addClip: i.model.addClip(); break;
        case ShellAction::remove: if (i.model.selectedClip() != 0) i.model.removeClip(); else i.model.removeTrack(); break;
        case ShellAction::duplicate: i.model.duplicateClip(); break;
        case ShellAction::undo: i.model.undo(); break;
        case ShellAction::redo: i.model.redo(); break;
        case ShellAction::browser: i.showBrowser = !i.showBrowser; break;
        case ShellAction::mixer: i.showMixer = !i.showMixer; break;
        case ShellAction::inspector: i.showInspector = !i.showInspector; break;
        case ShellAction::help: i.helpOpen = !i.helpOpen; break;
    }
    i.refresh();
}
bool WorkspaceView::keyPressed(const juce::KeyPress& key) {
    if (dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent()) != nullptr) return false;
    if (impl_->helpOpen) { if (key.getKeyCode() == juce::KeyPress::escapeKey) perform(ShellAction::help); return true; }
    if (key.getModifiers().isCtrlDown()) {
        const auto c = key.getKeyCode();
        if (c == 'N') perform(ShellAction::newSession);
        else if (c == 'Z') perform(ShellAction::undo);
        else if (c == 'Y') perform(ShellAction::redo);
        else if (c == 'D') perform(ShellAction::duplicate);
        else if (c == 'S') { impl_->status = "Saving is not available yet. This sketch exists only in memory."; repaint(); }
        else return false;
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::spaceKey) { perform(ShellAction::playPause); return true; }
    if (key.getKeyCode() == juce::KeyPress::returnKey) { perform(ShellAction::stop); return true; }
    if (key.getKeyCode() == juce::KeyPress::deleteKey) { perform(ShellAction::remove); return true; }
    switch (key.getKeyCode()) {
        case 'B': perform(ShellAction::browser); break;
        case 'M': perform(ShellAction::mixer); break;
        case 'I': perform(ShellAction::inspector); break;
        case '?': perform(ShellAction::help); break;
        default: return false;
    }
    return true;
}
bool WorkspaceView::keyPressed(const juce::KeyPress& key, juce::Component* source) {
    // Preserve Enter activation on focused native controls; Space/commands remain global.
    if (key.getKeyCode() == juce::KeyPress::returnKey && source != this) return false;
    return keyPressed(key);
}
juce::String WorkspaceView::runSmokeTest() {
    auto& i = *impl_;
    auto click = [this](const char* id) { auto* b = dynamic_cast<juce::TextButton*>(findChildWithID(id)); if (b == nullptr || !b->isEnabled()) return false; b->onClick(); return true; };
    if (i.model.tracks().size() != 4 || !click("new") || !i.model.tracks().empty()) return "Initial/new session failed";
    if (!click("add-track") || !click("add-clip") || i.model.tracks().size() != 1 || i.model.selectedClip() == 0) return "Native add controls failed";
    if (!click("duplicate") || i.model.tracks()[0].clips.size() != 2 || !click("remove-clip") || i.model.tracks()[0].clips.size() != 1) return "Clip controls failed";
    i.trackName.setText("Renamed", false); i.trackName.onReturnKey(); if (i.model.tracks()[0].name != "Renamed") return "Track rename failed";
    i.tempo.setText("0", false); i.tempo.onReturnKey(); if (i.model.bpm() != 120) return "Invalid tempo accepted";
    i.tempo.setText("96", false); i.tempo.onReturnKey(); if (i.model.bpm() != 96) return "Tempo edit failed";
    if (!click("mute") || !i.model.tracks()[0].muted || !click("solo") || !i.model.tracks()[0].solo) return "Mute/solo controls failed";
    i.level.setValue(-12, juce::sendNotificationSync); if (i.model.tracks()[0].level != -12) return "Level control failed";
    i.snap.setSelectedId(1, juce::sendNotificationSync); if (i.model.grid != 4) return "Snap control failed";
    i.search.setText("nonexistent", false); i.search.onTextChange(); if (i.starter.isVisible() || i.patterned.isVisible()) return "Browser filter failed";
    i.search.setText({}, false); i.search.onTextChange();
    if (!keyPressed(juce::KeyPress('B')) || i.showBrowser || !keyPressed(juce::KeyPress('B')) || !i.showBrowser) return "Browser shortcut failed";
    perform(ShellAction::mixer); if (i.mixerView.isVisible()) return "Mixer toggle failed"; perform(ShellAction::mixer);
    perform(ShellAction::inspector); if (i.trackName.isVisible()) return "Inspector toggle failed"; perform(ShellAction::inspector);
    perform(ShellAction::playPause); if (!i.model.playing()) return "Visual play failed"; perform(ShellAction::stop);
    perform(ShellAction::undo); perform(ShellAction::redo); perform(ShellAction::help); if (!i.helpPanel.isVisible()) return "Help failed"; perform(ShellAction::help);
    for (const auto size : {juce::Point<int>{1080, 640}, juce::Point<int>{1080, 700}, juce::Point<int>{1440, 900}, juce::Point<int>{1920, 1080}}) {
        setSize(size.x, size.y);
        if (i.lanes.getWidth() < 400 || i.lanes.getHeight() < 200 || !getLocalBounds().contains(i.inspectorArea) || i.addTrack.getBounds().intersects(i.zoomOut.getBounds())) return "Responsive layout failed";
        auto image = createComponentSnapshot(getLocalBounds()); if (!image.isValid()) return "Component render failed";
    }
    perform(ShellAction::demo);
    // Feed the same JUCE mouse handlers used by real clip gestures.
    const auto firstId = i.model.tracks()[0].clips[0].id;
    const auto down = i.canvas.clipBounds(i.model.tracks()[0].clips[0], 0).getCentre();
    auto eventAt = [&i, down](juce::Point<float> point) {
        return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(), point,
            juce::ModifierKeys::leftButtonModifier, 1.0F, 0, 0, 0, 0, &i.canvas, &i.canvas,
            juce::Time::getCurrentTime(), down, juce::Time::getCurrentTime(), 1, true);
    };
    i.canvas.mouseDown(eventAt(down));
    const auto destination = down.translated(static_cast<float>(4 * i.canvas.beatWidth), 0);
    i.canvas.mouseDrag(eventAt(destination)); i.canvas.mouseUp(eventAt(destination));
    if (i.model.clip(firstId)->beat != 4 || i.model.selectedClip() != firstId) return "Clip drag binding failed";
    perform(ShellAction::undo); if (i.model.clip(firstId)->beat != 0) return "Clip drag undo failed";
    perform(ShellAction::demo); i.search.clear(); i.snap.setSelectedId(3, juce::sendNotificationSync); i.zoom = 1; setSize(1440, 900); i.refresh();
    return {};
}
} // namespace simplicity
