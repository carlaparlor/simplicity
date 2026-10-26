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
PatternPanel::PatternPanel(Workspace& w):model(w) {
    setName("Patterns, reusable note sources"); setComponentID("patterns-pane");
    for(auto* c:std::initializer_list<juce::Component*>{&list,&name,&add,&clone,&unique,&remove}) addAndMakeVisible(c);
    list.setMultipleSelectionEnabled(false);list.setRowHeight(68); list.setColour(juce::ListBox::backgroundColourId,ui::background);
    name.setFont(ui::font()); name.setEditable(false,true); name.setName("Pattern name"); name.onTextChange=[this]{model.renamePattern(name.getText().toStdString()); if(changed)changed();};
    add.setComponentID("new-pattern"); clone.setComponentID("clone-pattern"); unique.setComponentID("unique-pattern"); remove.setComponentID("delete-pattern");
    add.setTooltip("New pattern (F4)"); clone.setTooltip("Clone pattern source"); unique.setTooltip("Make selected placement unique"); remove.setTooltip("Delete unused pattern; sources in use are protected");
    add.onClick=[this]{model.addPattern("Pattern "+std::to_string(model.patterns().size()+1)); if(changed)changed();};
    clone.onClick=[this]{model.clonePattern(); if(changed)changed();};
    unique.onClick=[this]{model.makeUnique(); if(changed)changed();};
    remove.onClick=[this]{model.deletePattern(); if(changed)changed();};
}
juce::var PatternPanel::getDragSourceDescription(const juce::SparseSet<int>& rows){
    if(rows.size()==0)return {};
    const int row=rows[0];if(row<0 || row>=getNumRows())return {};
    return "pattern:"+juce::String(model.patterns()[static_cast<std::size_t>(row)].id);
}
int PatternPanel::getNumRows(){return static_cast<int>(model.patterns().size());}
void PatternPanel::paintListBoxItem(int row,juce::Graphics& g,int w,int h,bool selected) {
    if(row<0 || row>=getNumRows())return;
    const auto& p=model.patterns()[static_cast<std::size_t>(row)]; const auto col=colours[p.colour%4];
    const juce::Rectangle<float> r{5,3,static_cast<float>(w-10),static_cast<float>(h-6)};
    g.setColour(selected?ui::selected:ui::raised); g.fillRoundedRectangle(r,3);
    g.setColour(col); g.fillRect(5,6,2,h-12);
    ui::label(g,juce::String(row+1).paddedLeft('0',2)+"  "+p.name,{14,7,w-24,21});
    drawPatternNotes(g,p,r.reduced(12,8).withTrimmedTop(20),0,static_cast<double>(p.length)/96,col);
    if(selected){g.setColour(ui::focus);g.drawRoundedRectangle(r,3,1);}
}
void PatternPanel::selectedRowsChanged(int row){if(row>=0 && row<getNumRows()){model.selectPattern(model.patterns()[static_cast<std::size_t>(row)].id);if(changed)changed();}}
void PatternPanel::listBoxItemDoubleClicked(int,const juce::MouseEvent&){if(edit)edit();}
void PatternPanel::refresh(){
    list.updateContent(); int selected=-1;
    for(std::size_t n=0;n<model.patterns().size();++n)if(model.patterns()[n].id==model.activePattern())selected=static_cast<int>(n);
    list.selectRow(selected,true,false); list.repaint();
    if(const auto* p=model.pattern(model.activePattern())){if(!name.isBeingEdited())name.setText(p->name,juce::dontSendNotification);}
    else name.setText({},juce::dontSendNotification);
    add.setEnabled(model.patterns().size()<Workspace::maxPatterns); clone.setEnabled(selected>=0 && add.isEnabled()); unique.setEnabled(model.selectedClip()!=0 && add.isEnabled()); remove.setEnabled(selected>=0);
}
void PatternPanel::beginRename(){name.showEditor();}
void PatternPanel::paint(juce::Graphics& g){g.fillAll(ui::panel);g.setColour(ui::border);g.drawVerticalLine(0,0,static_cast<float>(getHeight()));ui::label(g,"PATTERNS",{14,7,getWidth()-28,26},14);}
void PatternPanel::resized(){name.setBounds(10,37,getWidth()-20,28);list.setBounds(5,106,getWidth()-10,getHeight()-116);int x=12;for(auto* b:{&add,&clone,&unique,&remove}){b->setBounds(x,70,32,28);x+=40;}}
PatternEditor::PatternEditor(Workspace& w):model(w){
    setName("Pattern note editor");setComponentID("pattern-editor");setWantsKeyboardFocus(true);addAndMakeVisible(channel);
    channel.addItem("Kick",1);channel.addItem("Snare",2);channel.addItem("Hat",3);channel.addItem("Synth",4);channel.setSelectedId(4,juce::dontSendNotification);channel.setName("MIDI channel slot");channel.onChange=[this]{repaint();};
}
void PatternEditor::resized(){channel.setBounds(getWidth()-150,6,138,27);}
void PatternEditor::refresh(){
    channel.setVisible(piano);const auto* p=model.pattern(model.activePattern());
    if(p && p->id!=shownPattern){shownPattern=p->id;if(!p->notes.empty()){basePitch=std::clamp(p->notes.front().pitch/12*12,0,116);channel.setSelectedId(p->notes.front().channel+1,juce::dontSendNotification);}}
    repaint();
}
juce::Rectangle<int> PatternEditor::gridBounds() const{return getLocalBounds().withTrimmedTop(43).withTrimmedLeft(86).reduced(0,8).withTrimmedRight(14);}
void PatternEditor::paint(juce::Graphics& g){
    g.fillAll(ui::panel);const auto* p=model.pattern(model.activePattern());
    ui::label(g,(piano?"PIANO ROLL  /  ":"CHANNEL RACK  /  ")+(p?juce::String(p->name):"No pattern"),{14,4,getWidth()-180,30});
    if(!p)return;
    const auto r=gridBounds();const int rows=piano?12:4;const float rh=static_cast<float>(r.getHeight())/static_cast<float>(rows);const float cw=static_cast<float>(r.getWidth())/static_cast<float>(p->length/24);
    const char* names[]{"Kick","Snare","Hat","Synth"};
    for(int row=0;row<rows;++row){const int y=r.getY()+static_cast<int>(row*rh);g.setColour(row%2==0?ui::background:juce::Colour(0xff1d1d1d));g.fillRect(r.getX(),y,r.getWidth(),static_cast<int>(std::ceil(rh)));ui::label(g,piano?juce::String(basePitch+11-row):names[row],{12,y,70,static_cast<int>(rh)},piano?12.0F:14.0F,ui::secondary);}
    for(int step=0;step<p->length/24;++step){const int x=r.getX()+static_cast<int>(step*cw);g.setColour(step%4==0?ui::border:ui::border.withAlpha(0.35F));g.drawVerticalLine(x,static_cast<float>(r.getY()),static_cast<float>(r.getBottom()));}
    for(const auto& n:p->notes){if(piano && n.channel!=channel.getSelectedId()-1)continue;const int row=piano?basePitch+11-n.pitch:n.channel;if(row<0 || row>=rows)continue;const float x=static_cast<float>(r.getX())+static_cast<float>(n.tick)/24*cw;const float y=static_cast<float>(r.getY())+static_cast<float>(row)*rh+2;g.setColour(colours[n.channel]);g.fillRoundedRectangle(x+1,y,std::max(2.0F,static_cast<float>(n.length)/24*cw-2),std::max(2.0F,rh-4),1);}
}
bool PatternEditor::editCell(int step,int row,bool erase){
    const auto* p=model.pattern(model.activePattern());if(!p || step<0 || step>=p->length/24 || row<0 || row>=(piano?12:4))return false;
    const int ch=piano?channel.getSelectedId()-1:row;const int pitches[]{36,38,42,60};const int pitch=piano?basePitch+11-row:pitches[ch];
    for(std::size_t n=0;n<p->notes.size();++n)if((piano?(p->notes[n].tick<=step*24 && step*24<p->notes[n].tick+p->notes[n].length):p->notes[n].tick==step*24) && p->notes[n].channel==ch && (!piano || p->notes[n].pitch==pitch)){const bool ok=model.removeNote(static_cast<int>(n));if(changed)changed();return ok;}
    if(erase)return false;
    const bool ok=model.addNote({step*24,24,pitch,ch,100});if(changed)changed();return ok;
}
void PatternEditor::mouseDown(const juce::MouseEvent& e){
    grabKeyboardFocus();const auto* p=model.pattern(model.activePattern());const auto r=gridBounds();
    if(!p || !r.contains(e.getPosition()))return;
    const int step=(e.x-r.getX())*(p->length/24)/r.getWidth();const int row=(e.y-r.getY())*(piano?12:4)/r.getHeight();
    gesturePattern=p->id;heldNote=-1;lastStep=step;held=row;stretching=piano && !e.mods.isRightButtonDown();
    if(!stretching)editCell(step,row,e.mods.isRightButtonDown());
    else {for(std::size_t n=0;n<p->notes.size();++n)if(p->notes[n].pitch==basePitch+11-row && p->notes[n].channel==channel.getSelectedId()-1 && p->notes[n].tick<=step*24 && step*24<p->notes[n].tick+p->notes[n].length){heldNote=static_cast<int>(n);lastStep=p->notes[n].tick/24;break;}} // One duration edit per gesture.
}
void PatternEditor::mouseDrag(const juce::MouseEvent& e){
    if(held<0)return;
    const auto* p=model.pattern(model.activePattern());if(!p)return;
    const auto r=gridBounds();const int step=std::clamp((e.x-r.getX())*(p->length/24)/r.getWidth(),0,p->length/24-1);
    if(!stretching && step!=lastStep){lastStep=step;editCell(step,held,e.mods.isRightButtonDown());}
}
void PatternEditor::mouseUp(const juce::MouseEvent& e){
    if(stretching && held>=0 && model.activePattern()==gesturePattern){const auto* p=model.pattern(model.activePattern());if(p){const auto r=gridBounds();const int end=std::clamp((e.x-r.getX())*(p->length/24)/r.getWidth(),lastStep,p->length/24-1);if(heldNote>=0)model.resizeNote(heldNote,(end-lastStep+1)*24);else model.addNote({lastStep*24,(end-lastStep+1)*24,basePitch+11-held,channel.getSelectedId()-1,100});if(changed)changed();}}
    held=-1;stretching=false;
}
void PatternEditor::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& wheel){if(piano){basePitch=std::clamp(basePitch+(wheel.deltaY>0?12:-12),0,116);repaint();}}
}
