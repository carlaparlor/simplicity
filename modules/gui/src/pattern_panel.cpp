#include <simplicity/pattern_panel.h>
#include <algorithm>
#include <cmath>

namespace simplicity {
namespace {
const juce::Colour colours[]{juce::Colour{0xffa4c98e},juce::Colour{0xffcba57e},juce::Colour{0xff8daabf},juce::Colour{0xffb1a0c9}};
}
void drawPatternNotes(juce::Graphics& g,const Pattern& p,juce::Rectangle<float> r,double offset,double beats,juce::Colour colour) {
    if(p.notes.empty() || r.isEmpty() || beats<=0) return;
    int low=127,high=0;
    for(const auto& n:p.notes) { low=std::min(low,n.pitch); high=std::max(high,n.pitch); }
    const int span=std::max(12,high-low+3);
    const double source=static_cast<double>(p.length)/Workspace::ticksPerBeat;
    juce::Graphics::ScopedSaveState saved(g); g.reduceClipRegion(r.toNearestInt());
    g.setColour(colour);
    const int first=static_cast<int>(std::floor(offset/source));
    const int last=static_cast<int>(std::ceil((offset+beats)/source));
    for(int repeat=first;repeat<last;++repeat) for(const auto& n:p.notes) {
        const double at=repeat*source+static_cast<double>(n.tick)/Workspace::ticksPerBeat-offset;
        const float x=r.getX()+static_cast<float>(at/beats)*r.getWidth();
        const float w=std::max(1.0F,static_cast<float>(n.length)/Workspace::ticksPerBeat/static_cast<float>(beats)*r.getWidth());
        const float y=r.getBottom()-static_cast<float>(n.pitch-low+2)/static_cast<float>(span)*r.getHeight();
        g.fillRect(x,y,w,std::clamp(r.getHeight()/static_cast<float>(span),1.0F,3.0F));
    }
}
juce::Colour patternColour(int index){return colours[static_cast<unsigned int>(index)%4];}
void drawPatternClip(juce::Graphics& g,const Pattern& p,juce::Rectangle<float> r,double offset,double beats,bool selected,bool quiet){
    const auto colour=patternColour(p.colour);
    g.setColour(juce::Colours::black.withAlpha(0.5F));g.fillRoundedRectangle(r.translated(1,2),2);
    g.setColour(colour.interpolatedWith(ui::background,quiet?0.93F:0.78F));g.fillRoundedRectangle(r,2);
    g.setGradientFill(juce::ColourGradient(colour.withAlpha(quiet?0.12F:0.5F),r.getX(),r.getY(),colour.withAlpha(quiet?0.08F:0.28F),r.getX(),r.getY()+18,false));g.fillRect(r.withHeight(18));
    ui::label(g,p.name,r.toNearestInt().reduced(5,0).withHeight(18),12,quiet?ui::secondary:ui::foreground);
    drawPatternNotes(g,p,r.reduced(3,3).withTrimmedTop(18),offset,beats,colour.withAlpha(quiet?0.25F:0.95F));
    g.setColour(selected?ui::foreground:colour.withAlpha(0.3F));g.drawRoundedRectangle(r,2,1);
}
class PatternPanel::Card final : public juce::Button {
public:
    PatternPanel& owner;int id;bool dragging{};
    Card(PatternPanel& p,int source):juce::Button("Pattern"),owner(p),id(source){
        setComponentID("pattern-card-"+juce::String(id));
        onClick=[this]{owner.model.selectPattern(id);if(owner.changed)owner.changed();};
    }
    void paintButton(juce::Graphics& g,bool,bool) override {if(const auto* p=owner.model.pattern(id))drawPatternClip(g,*p,getLocalBounds().toFloat().reduced(3),0,static_cast<double>(p->length)/96,owner.model.activePattern()==id || hasKeyboardFocus(false));}
    void mouseDown(const juce::MouseEvent& e) override {dragging=false;juce::Button::mouseDown(e);if(onClick)onClick();}
    void mouseDrag(const juce::MouseEvent& e) override {
        juce::Button::mouseDrag(e);
        if(!dragging && e.mods.isLeftButtonDown() && e.getDistanceFromDragStart()>5)if(auto* container=juce::DragAndDropContainer::findParentDragContainerFor(this)){dragging=true;container->startDragging("pattern:"+juce::String(id),this);}
    }
    void mouseDoubleClick(const juce::MouseEvent&) override {if(onClick)onClick();if(owner.edit)owner.edit();}
};
PatternPanel::PatternPanel(Workspace& w):model(w){
    setName("Patterns, reusable note sources");setComponentID("patterns-pane");
    for(auto* c:std::initializer_list<juce::Component*>{&list,&name,&add,&clone,&unique,&remove})addAndMakeVisible(c);
    list.setViewedComponent(&content,false);list.setScrollBarsShown(true,false);list.setScrollBarThickness(7);
    name.setFont(ui::font());name.setEditable(false,true);name.setName("Pattern name");name.onTextChange=[this]{model.renamePattern(name.getText().toStdString());if(changed)changed();};
    add.setComponentID("new-pattern");clone.setComponentID("clone-pattern");unique.setComponentID("unique-pattern");remove.setComponentID("delete-pattern");
    add.setTooltip("New pattern (F4)");clone.setTooltip("Clone pattern source");unique.setTooltip("Make selected placement unique");remove.setTooltip("Delete unused pattern; sources in use are protected");
    add.onClick=[this]{model.addPattern("Pattern "+std::to_string(model.patterns().size()+1));if(changed)changed();};
    clone.onClick=[this]{model.clonePattern();if(changed)changed();};unique.onClick=[this]{model.makeUnique();if(changed)changed();};remove.onClick=[this]{model.deletePattern();if(changed)changed();};
}
PatternPanel::~PatternPanel(){list.setViewedComponent(nullptr,false);}
juce::Rectangle<int> PatternPanel::cardBounds(int index) const {return juce::isPositiveAndBelow(index,cards.size())?cards[index]->getBounds():juce::Rectangle<int>{};}
void PatternPanel::refresh(){
    bool rebuild=cards.size()!=static_cast<int>(model.patterns().size());
    if(!rebuild)for(int n=0;n<cards.size();++n)if(cards[n]->id!=model.patterns()[static_cast<std::size_t>(n)].id)rebuild=true;
    if(rebuild){cards.clear();for(const auto& p:model.patterns()){auto* card=cards.add(new Card(*this,p.id));content.addAndMakeVisible(card);}}
    int selected=-1;
    for(int n=0;n<cards.size();++n){const auto& p=model.patterns()[static_cast<std::size_t>(n)];cards[n]->setName("Pattern "+juce::String(n+1)+": "+p.name);cards[n]->setTooltip(p.name);cards[n]->repaint();if(p.id==model.activePattern())selected=n;}
    if(const auto* p=model.pattern(model.activePattern())){if(!name.isBeingEdited())name.setText(p->name,juce::dontSendNotification);}else name.setText({},juce::dontSendNotification);
    add.setEnabled(model.patterns().size()<Workspace::maxPatterns);clone.setEnabled(selected>=0&&add.isEnabled());unique.setEnabled(model.selectedClip()!=0&&add.isEnabled());remove.setEnabled(selected>=0);
    resized();
    if(selected>=0 && !cards[selected]->getBounds().intersects(list.getViewArea()))list.setViewPosition(0,cards[selected]->getY());
}
void PatternPanel::beginRename(){name.showEditor();}
void PatternPanel::paint(juce::Graphics& g){g.fillAll(ui::panel);g.setColour(ui::border);g.drawVerticalLine(0,0,static_cast<float>(getHeight()));ui::label(g,"PATTERNS",{14,7,getWidth()-28,26},14);}
void PatternPanel::resized(){
    name.setBounds(10,37,getWidth()-20,28);list.setBounds(5,106,getWidth()-10,std::max(1,getHeight()-116));
    const int width=std::max(1,(list.getWidth()-19)/2);content.setSize(std::max(1,list.getWidth()-7),((cards.size()+1)/2)*58+6);
    for(int n=0;n<cards.size();++n)cards[n]->setBounds(3+(n%2)*(width+6),3+(n/2)*58,width,52);
    int x=12;for(auto* b:{&add,&clone,&unique,&remove}){b->setBounds(x,70,32,28);x+=40;}
}
PatternEditor::PatternEditor(Workspace& w):model(w){
    setName("Pattern note editor");setComponentID("pattern-editor");setWantsKeyboardFocus(true);
    for(auto* c:std::initializer_list<juce::Component*>{&channel,&timeZoom,&verticalZoom,&pitchScroll})addAndMakeVisible(c);
    channel.addItem("Kick",1);channel.addItem("Snare",2);channel.addItem("Hat",3);channel.addItem("Synth",4);channel.setSelectedId(4,juce::dontSendNotification);channel.setName("MIDI channel slot");channel.onChange=[this]{held=-1;repaint();};
    timeZoom.changed=[this]{held=-1;repaint();};verticalZoom.scale=[this](double factor){zoomVertical(factor);};verticalZoom.reset=[this]{pitchRows=12;basePitch=std::min(basePitch,116);syncPitchScroll();repaint();};
    pitchScroll.setRangeLimits(0,128,juce::dontSendNotification);pitchScroll.setName("Piano pitch scroll");pitchScroll.addListener(this);
}
PatternEditor::~PatternEditor(){pitchScroll.removeListener(this);}
void PatternEditor::resized(){channel.setBounds(getWidth()-150,6,138,27);const auto r=gridBounds();timeZoom.setBounds(r.getX(),43,r.getWidth(),16);verticalZoom.setBounds(getWidth()-18,43,16,16);pitchScroll.setBounds(getWidth()-17,r.getY(),14,r.getHeight());}
void PatternEditor::syncPitchScroll(){pitchScroll.setCurrentRange(128-basePitch-pitchRows,pitchRows,juce::dontSendNotification);}
void PatternEditor::scrollBarMoved(juce::ScrollBar*,double start){held=-1;basePitch=std::clamp(128-pitchRows-static_cast<int>(std::round(start)),0,128-pitchRows);repaint();}
void PatternEditor::refresh(){
    channel.setVisible(piano);timeZoom.setVisible(piano);verticalZoom.setVisible(piano);pitchScroll.setVisible(piano);
    const auto* p=model.pattern(model.activePattern());if(!p)shownPattern=0;
    if(p && p->id!=shownPattern){held=-1;shownPattern=p->id;timeZoom.setRange(0,1);if(!p->notes.empty()){basePitch=std::clamp(p->notes.front().pitch/12*12,0,128-pitchRows);channel.setSelectedId(p->notes.front().channel+1,juce::dontSendNotification);}}
    syncPitchScroll();resized();repaint();
}
juce::Rectangle<int> PatternEditor::gridBounds() const{return getLocalBounds().withTrimmedTop(piano?64:43).withTrimmedLeft(86).withTrimmedRight(20).withTrimmedBottom(12);}
void PatternEditor::zoomHorizontal(double factor,double anchor){if(piano)timeZoom.zoom(factor,anchor);}
void PatternEditor::zoomVertical(double factor){
    if(!piano || !std::isfinite(factor)||factor<=0)return;
    const int centre=basePitch+pitchRows/2;pitchRows=std::clamp(static_cast<int>(std::round(pitchRows/factor)),6,48);basePitch=std::clamp(centre-pitchRows/2,0,128-pitchRows);held=-1;syncPitchScroll();repaint();
}
bool PatternEditor::keyPressed(const juce::KeyPress& key){
    if(!piano)return false;
    const int code=key.getKeyCode();
    if(code==juce::KeyPress::pageUpKey||code==juce::KeyPress::pageDownKey){zoomHorizontal(code==juce::KeyPress::pageUpKey?1.25:0.8,timeZoom.start()+timeZoom.span()/2);return true;}
    if(key.getModifiers().isShiftDown() && code>='1' && code<='4'){timeZoom.setRange(0,code=='4'?1:1.0/(1<<(code-'1'+1)));held=-1;repaint();return true;}
    return false;
}
int PatternEditor::stepAt(int x) const{
    const auto* p=model.pattern(model.activePattern());if(!p)return 0;const auto r=gridBounds();const int total=p->length/24;
    const double time=(piano?timeZoom.start():0)+(static_cast<double>(x-r.getX())/std::max(1,r.getWidth()))*(piano?timeZoom.span():1);
    return std::clamp(static_cast<int>(std::floor(time*total)),0,total-1);
}
void PatternEditor::paint(juce::Graphics& g){
    g.fillAll(ui::panel);const auto* p=model.pattern(model.activePattern());ui::label(g,(piano?"PIANO ROLL  /  ":"CHANNEL RACK  /  ")+(p?juce::String(p->name):"No pattern"),{14,4,getWidth()-180,30});if(!p)return;
    const auto r=gridBounds();if(r.isEmpty())return;const int rows=visibleRows();const float rh=static_cast<float>(r.getHeight())/static_cast<float>(rows);const float cw=static_cast<float>(r.getWidth())/static_cast<float>(p->length/24)/(piano?static_cast<float>(timeZoom.span()):1.0F);
    const float offset=piano?static_cast<float>(timeZoom.start()*(p->length/24))*cw:0;
    const char* names[]{"Kick","Snare","Hat","Synth"};
    for(int row=0;row<rows;++row){const int y=r.getY()+static_cast<int>(static_cast<float>(row)*rh);g.setColour(row%2==0?ui::background:juce::Colour(0xff1d1d1d));g.fillRect(r.getX(),y,r.getWidth(),static_cast<int>(std::ceil(rh)));if(!piano || rh>=12 || (basePitch+rows-1-row)%12==0)ui::label(g,piano?juce::String(basePitch+rows-1-row):names[row],{12,y,70,static_cast<int>(rh)},piano?12.0F:14.0F,ui::secondary);}
    juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(r);
    for(int step=0;step<=p->length/24;++step){const int x=r.getX()+static_cast<int>(static_cast<float>(step)*cw-offset);g.setColour(step%4==0?ui::border:ui::border.withAlpha(0.35F));g.drawVerticalLine(x,static_cast<float>(r.getY()),static_cast<float>(r.getBottom()));}
    for(const auto& n:p->notes){if(piano && n.channel!=channel.getSelectedId()-1)continue;const int row=piano?basePitch+rows-1-n.pitch:n.channel;if(row<0||row>=rows)continue;const float x=static_cast<float>(r.getX())+static_cast<float>(n.tick)/24*cw-offset;const float y=static_cast<float>(r.getY())+static_cast<float>(row)*rh+2;g.setColour(colours[n.channel]);g.fillRoundedRectangle(x+1,y,std::max(2.0F,static_cast<float>(n.length)/24*cw-2),std::max(2.0F,rh-4),1);}
}
bool PatternEditor::editCell(int step,int row,bool erase){
    const auto* p=model.pattern(model.activePattern());if(!p||step<0||step>=p->length/24||row<0||row>=visibleRows())return false;
    const int ch=piano?channel.getSelectedId()-1:row;const int pitches[]{36,38,42,60};const int pitch=piano?basePitch+pitchRows-1-row:pitches[ch];
    for(std::size_t n=0;n<p->notes.size();++n)if((piano?(p->notes[n].tick<=step*24 && step*24<p->notes[n].tick+p->notes[n].length):p->notes[n].tick==step*24) && p->notes[n].channel==ch && (!piano||p->notes[n].pitch==pitch)){const bool ok=model.removeNote(static_cast<int>(n));if(changed)changed();return ok;}
    if(erase)return false;
    const bool ok=model.addNote({step*24,24,pitch,ch,100});if(changed)changed();return ok;
}
void PatternEditor::mouseDown(const juce::MouseEvent& e){
    grabKeyboardFocus();panning=e.mods.isMiddleButtonDown();held=-1;
    if(panning){panStart=timeZoom.start();panPitch=basePitch;return;}
    const auto* p=model.pattern(model.activePattern());const auto r=gridBounds();if(!p||r.isEmpty()||!r.contains(e.getPosition()))return;
    const int step=stepAt(e.x);const int row=(e.y-r.getY())*visibleRows()/r.getHeight();gesturePattern=p->id;heldNote=-1;lastStep=step;held=row;stretching=piano&&!e.mods.isRightButtonDown();
    if(!stretching)editCell(step,row,e.mods.isRightButtonDown());
    else for(std::size_t n=0;n<p->notes.size();++n)if(p->notes[n].pitch==basePitch+pitchRows-1-row && p->notes[n].channel==channel.getSelectedId()-1 && p->notes[n].tick<=step*24 && step*24<p->notes[n].tick+p->notes[n].length){heldNote=static_cast<int>(n);lastStep=p->notes[n].tick/24;break;}
}
void PatternEditor::mouseDrag(const juce::MouseEvent& e){
    if(panning && piano){const auto r=gridBounds();timeZoom.setRange(panStart-static_cast<double>(e.getDistanceFromDragStartX())/std::max(1,r.getWidth())*timeZoom.span(),timeZoom.span());basePitch=std::clamp(panPitch+e.getDistanceFromDragStartY()*pitchRows/std::max(1,r.getHeight()),0,128-pitchRows);syncPitchScroll();repaint();return;}
    if(held<0||!model.pattern(model.activePattern()))return;
    const int step=stepAt(e.x);if(!stretching&&step!=lastStep){lastStep=step;editCell(step,held,e.mods.isRightButtonDown());}
}
void PatternEditor::mouseUp(const juce::MouseEvent& e){
    if(stretching && held>=0 && model.activePattern()==gesturePattern){if(model.pattern(model.activePattern())){const int end=std::max(lastStep,stepAt(e.x));if(heldNote>=0){if(e.mouseWasDraggedSinceMouseDown())model.resizeNote(heldNote,(end-lastStep+1)*24);}else model.addNote({lastStep*24,(end-lastStep+1)*24,basePitch+pitchRows-1-held,channel.getSelectedId()-1,100});if(changed)changed();}}
    held=-1;stretching=false;panning=false;
}
void PatternEditor::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& wheel){
    if(!piano||wheel.deltaY==0)return;
    if(e.mods.isCtrlDown()){const auto r=gridBounds();zoomHorizontal(wheel.deltaY>0?1.25:0.8,timeZoom.start()+std::clamp(static_cast<double>(e.x-r.getX())/std::max(1,r.getWidth()),0.0,1.0)*timeZoom.span());}
    else if(e.mods.isAltDown())zoomVertical(wheel.deltaY>0?1.25:0.8);
    else {held=-1;basePitch=std::clamp(basePitch+(wheel.deltaY>0?12:-12),0,128-pitchRows);syncPitchScroll();repaint();}
}
}
