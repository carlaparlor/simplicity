#include <simplicity/browser_panel.h>

namespace simplicity {
namespace {
const char* templateNames[]{"Empty track", "Pattern track", "Rhythm track", "Texture track"};
const ui::Glyph templateIcons[]{ui::Glyph::tracks, ui::Glyph::pattern, ui::Glyph::rhythm, ui::Glyph::texture};
}
BrowserPanel::BrowserPanel(Workspace& model) : model_(model) {
    setName("Session browser"); setComponentID("browser-panel");
    for (auto* c : std::initializer_list<juce::Component*>{&session_, &templatesButton_, &search, &viewport_, &add_}) addAndMakeVisible(c);
    session_.setComponentID("browser-session"); templatesButton_.setComponentID("browser-templates"); add_.setComponentID("browser-new-track");
    session_.onClick = [this] { setTemplateTab(false); }; templatesButton_.onClick = [this] { setTemplateTab(true); };
    session_.setTooltip("Browse and select session tracks"); templatesButton_.setTooltip("Add in-memory track templates; no audio or instruments included");
    search.setName("Search browser"); search.setComponentID("browser-search"); search.setFont(ui::font(13));
    search.setTextToShowWhenEmpty("Search", ui::secondary); search.setBorder({6, 31, 5, 8});
    search.onTextChange = [this] { updateResults(); };
    viewport_.setViewedComponent(&content_, false); viewport_.setScrollBarsShown(true, false); viewport_.setScrollBarThickness(6);
    for (int n = 0; n < Workspace::maxTracks; ++n) {
        auto* row = rows_.add(new ui::IconButton({}, ui::Glyph::wave, true));
        row->setComponentID("browser-row-" + juce::String(n)); row->getProperties().set("flat", true); row->onClick = [this, n] { activateResult(n); };
        content_.addChildComponent(row);
    }
    add_.onClick = [this] { if (model_.addTrack("Track " + std::to_string(model_.tracks().size()+1)) && changed) changed(); };
    add_.setTooltip("Add an empty track"); refresh();
}
BrowserPanel::~BrowserPanel() { viewport_.setViewedComponent(nullptr, false); }
void BrowserPanel::setTemplateTab(bool enabled) { templates_ = enabled; search.setText({}, false); viewport_.setViewPosition(0, 0); updateResults(); }
void BrowserPanel::setQuery(const juce::String& query) { search.setText(query, false); updateResults(); }
void BrowserPanel::refresh() { add_.setEnabled(model_.tracks().size() < Workspace::maxTracks); updateResults(); }
void BrowserPanel::updateResults() {
    results_.clear(); const auto q = search.getText().trim();
    const int count = templates_ ? 4 : static_cast<int>(model_.tracks().size());
    for (int n = 0; n < count; ++n) {
        const juce::String name = templates_ ? juce::String(templateNames[n]) : juce::String(model_.tracks()[static_cast<std::size_t>(n)].name);
        if (name.containsIgnoreCase(q)) results_.push_back(n);
    }
    for (int n = 0; n < rows_.size(); ++n) {
        auto* row = rows_[n]; const bool visible = n < resultCount(); row->setVisible(visible);
        if (!visible) continue;
        const int index = results_[static_cast<std::size_t>(n)];
        const auto name = templates_ ? juce::String(templateNames[index]) : juce::String(model_.tracks()[static_cast<std::size_t>(index)].name);
        row->setButtonText(name); row->setName(name); row->symbol = templates_ ? templateIcons[index] : ui::Glyph::wave;
        row->setToggleState(!templates_ && model_.selectedTrack() == index, juce::dontSendNotification);
        row->setEnabled(!templates_ || model_.tracks().size() < Workspace::maxTracks);
        row->setTooltip(templates_ ? "Add " + name + " (visual template only)" : "Select " + name);
    }
    session_.setToggleState(!templates_, juce::dontSendNotification); templatesButton_.setToggleState(templates_, juce::dontSendNotification);
    resized(); repaint();
}
bool BrowserPanel::activateResult(int row) {
    if (row < 0 || row >= resultCount()) return false;
    const int index = results_[static_cast<std::size_t>(row)];
    bool ok = false;
    if (!templates_) ok = model_.selectTrack(index);
    else {
        ok = model_.addTrack(templateNames[index]);
        if (ok && index > 0) {
            const double oldPosition = model_.position(); model_.seek(0);
            const int blocks = index == 2 ? 4 : index == 3 ? 2 : 1;
            for (int n = 0; n < blocks; ++n) { model_.seek(n * 4.0); model_.addClip(); }
            model_.seek(oldPosition);
        }
    }
    if (ok && changed) changed();
    return ok;
}
void BrowserPanel::resized() {
    session_.setBounds(12, 12, 98, 32); templatesButton_.setBounds(114, 12, getWidth()-126, 32);
    search.setBounds(12, 55, getWidth()-24, 33);
    viewport_.setBounds(8, 106, getWidth()-16, std::max(0, getHeight()-164));
    content_.setSize(viewport_.getWidth()-6, std::max(viewport_.getHeight(), resultCount()*42));
    for (int n = 0; n < rows_.size(); ++n) rows_[n]->setBounds(4, n*42, content_.getWidth()-8, 37);
    add_.setBounds(12, getHeight()-44, getWidth()-24, 32);
}
void BrowserPanel::paint(juce::Graphics& g) {
    g.fillAll(ui::panel); g.setColour(ui::border); g.drawVerticalLine(getWidth()-1, 0, static_cast<float>(getHeight()));
    if (results_.empty()) ui::label(g, search.getText().isEmpty() ? "No tracks" : "No matches", {23, 120, getWidth()-46, 28}, 13, ui::secondary);
}
}
