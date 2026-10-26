#pragma once
#include <simplicity/ui_style.h>
#include <functional>

namespace simplicity {
// Message-thread-only normalized time window. No model/audio ownership; setters
// clamp invalid/oversized ranges and never invoke callbacks. Drag callbacks are
// synchronous, so the owner must outlive this control.
class TimelineZoomBar final : public juce::Component, public juce::SettableTooltipClient {
public:
    static constexpr double minimumSpan=1.0/32.0;
    TimelineZoomBar();
    std::function<void()> changed;
    void setRange(double start,double span);
    void zoom(double factor,double anchor);
    double start() const noexcept {return start_;}
    double span() const noexcept {return span_;}
    juce::Rectangle<float> handleBounds() const;
    void paint(juce::Graphics&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;
    bool keyPressed(const juce::KeyPress&) override;
private:
    double start_{},span_{1},downStart{},downEnd{};
    int edge{};
};
class VerticalZoomGrip final : public juce::Component, public juce::SettableTooltipClient {
public:
    VerticalZoomGrip();
    std::function<void(double)> scale;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override;
    std::function<void()> reset;
private:
    int last{};
};
}
