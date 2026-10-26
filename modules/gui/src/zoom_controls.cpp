#include <simplicity/zoom_controls.h>
#include <algorithm>
#include <cmath>

namespace simplicity {
TimelineZoomBar::TimelineZoomBar(){setName("Horizontal scroll and zoom");setComponentID("time-zoom-bar");setTooltip("Drag handle edges to zoom; centre to scroll; double-click to fit");setWantsKeyboardFocus(true);}
void TimelineZoomBar::setRange(double start,double span){
    if(!std::isfinite(start)||!std::isfinite(span))return;
    span_=std::clamp(span,minimumSpan,1.0);start_=std::clamp(start,0.0,1.0-span_);repaint();
}
void TimelineZoomBar::zoom(double factor,double anchor){
    if(!std::isfinite(factor)||factor<=0||!std::isfinite(anchor))return;
    const double at=std::clamp(anchor,start_,start_+span_);const double next=std::clamp(span_/factor,minimumSpan,1.0);
    setRange(at-(at-start_)/span_*next,next);if(changed)changed();
}
juce::Rectangle<float> TimelineZoomBar::handleBounds() const{return {static_cast<float>(start_)*getWidth(),2,static_cast<float>(span_)*getWidth(),static_cast<float>(std::max(1,getHeight()-4))};}
void TimelineZoomBar::paint(juce::Graphics& g){
    g.fillAll(ui::background);const auto r=handleBounds().reduced(0.5F,0);g.setColour(ui::selected);g.fillRoundedRectangle(r,2);
    g.setColour(hasKeyboardFocus(false)?ui::focus:ui::secondary);g.drawRoundedRectangle(r,2,1);
    for(const float x:{r.getX()+3,r.getRight()-4})g.drawVerticalLine(static_cast<int>(x),r.getY()+2,r.getBottom()-2);
}
void TimelineZoomBar::mouseMove(const juce::MouseEvent& e){const auto r=handleBounds();setMouseCursor(std::abs(e.position.x-r.getX())<6||std::abs(e.position.x-r.getRight())<6?juce::MouseCursor::LeftRightResizeCursor:r.contains(e.position)?juce::MouseCursor::DraggingHandCursor:juce::MouseCursor::NormalCursor);}
void TimelineZoomBar::mouseDown(const juce::MouseEvent& e){
    grabKeyboardFocus();const auto r=handleBounds();edge=std::abs(e.position.x-r.getX())<6?-1:std::abs(e.position.x-r.getRight())<6?1:0;
    if(!r.contains(e.position)&&edge==0){setRange(static_cast<double>(e.x)/std::max(1,getWidth())-span_/2,span_);if(changed)changed();}
    downStart=start_;downEnd=start_+span_;
}
void TimelineZoomBar::mouseDrag(const juce::MouseEvent& e){
    const double d=static_cast<double>(e.getDistanceFromDragStartX())/std::max(1,getWidth());
    if(edge<0){const double left=std::clamp(downStart+d,0.0,downEnd-minimumSpan);setRange(left,downEnd-left);}
    else if(edge>0)setRange(downStart,std::clamp(downEnd+d,downStart+minimumSpan,1.0)-downStart);
    else setRange(downStart+d,downEnd-downStart);
    if(changed)changed();
}
void TimelineZoomBar::mouseDoubleClick(const juce::MouseEvent&){setRange(0,1);if(changed)changed();}
void TimelineZoomBar::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& w){if(w.deltaY!=0)zoom(w.deltaY>0?1.25:0.8,start_+span_/2);}
bool TimelineZoomBar::keyPressed(const juce::KeyPress& key){
    if(key.getKeyCode()==juce::KeyPress::leftKey || key.getKeyCode()==juce::KeyPress::rightKey){setRange(start_+(key.getKeyCode()==juce::KeyPress::leftKey?-1:1)*span_/8,span_);if(changed)changed();return true;}
    if(key.getKeyCode()==juce::KeyPress::pageUpKey||key.getKeyCode()==juce::KeyPress::pageDownKey){zoom(key.getKeyCode()==juce::KeyPress::pageUpKey?1.25:0.8,start_+span_/2);return true;}
    return false;
}
VerticalZoomGrip::VerticalZoomGrip(){setName("Vertical zoom, drag up or down");setTooltip("Drag up/down: lane or key height; double-click: reset");setComponentID("vertical-zoom-grip");setWantsKeyboardFocus(true);setMouseCursor(juce::MouseCursor::UpDownResizeCursor);}
void VerticalZoomGrip::paint(juce::Graphics& g){g.fillAll(ui::raised);g.setColour(hasKeyboardFocus(false)?ui::focus:ui::secondary);const float cx=static_cast<float>(getWidth())/2;g.drawLine(cx,3,cx,static_cast<float>(getHeight()-3),1);g.drawLine(cx-3,6,cx,3,1);g.drawLine(cx,3,cx+3,6,1);g.drawLine(cx-3,static_cast<float>(getHeight()-6),cx,static_cast<float>(getHeight()-3),1);g.drawLine(cx,static_cast<float>(getHeight()-3),cx+3,static_cast<float>(getHeight()-6),1);}
void VerticalZoomGrip::mouseDown(const juce::MouseEvent&){grabKeyboardFocus();last=0;}
void VerticalZoomGrip::mouseDrag(const juce::MouseEvent& e){const int dy=e.getDistanceFromDragStartY();if(scale)scale(std::pow(2.0,static_cast<double>(last-dy)/96));last=dy;}
void VerticalZoomGrip::mouseDoubleClick(const juce::MouseEvent&){if(reset)reset();}
bool VerticalZoomGrip::keyPressed(const juce::KeyPress& k){if(k.getKeyCode()==juce::KeyPress::upKey||k.getKeyCode()==juce::KeyPress::downKey){if(scale)scale(k.getKeyCode()==juce::KeyPress::upKey?1.1:1/1.1);return true;}return false;}
}
