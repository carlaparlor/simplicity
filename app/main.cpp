#include <simplicity/workspace_view.h>
#include <algorithm>

namespace {
class Window final : public juce::DocumentWindow {
public:
    Window() : DocumentWindow("simplicity", juce::Colour{0xff111315}, allButtons) {
        setUsingNativeTitleBar(false);
        auto* workspace = new simplicity::WorkspaceView();
        setContentOwned(workspace, true); setLookAndFeel(&workspace->getLookAndFeel());
        setTitleBarHeight(26); setResizable(true, false); setResizeLimits(1080, 666, 3840, 2160);
        int width = 1440, height = 926;
        if (const auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()) {
            width = std::clamp(display->userArea.getWidth() - 32, 1080, 1440);
            height = std::clamp(display->userArea.getHeight() - 32, 666, 926);
        }
        centreWithSize(width, height); setVisible(true);
    }
    ~Window() override { setLookAndFeel(nullptr); }
    void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
};
class SimplicityApplication final : public juce::JUCEApplication {
public:
    const juce::String getApplicationName() override { return "Simplicity"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void initialise(const juce::String& commandLine) override {
        window_ = std::make_unique<Window>();
        const auto args = juce::StringArray::fromTokens(commandLine, true);
        if (args.contains("--smoke-test")) {
            const auto reportIndex = args.indexOf("--report");
            if (reportIndex < 0 || reportIndex + 1 >= args.size()) { setApplicationReturnValue(2); quit(); return; }
            report_ = juce::File::getCurrentWorkingDirectory().getChildFile(args[reportIndex + 1].unquoted());
            juce::Timer::callAfterDelay(150, [this] {
                auto* view = dynamic_cast<simplicity::WorkspaceView*>(window_->getContentComponent());
                auto error = view != nullptr ? view->runSmokeTest() : juce::String("Workspace missing");
                juce::DynamicObject::Ptr result = new juce::DynamicObject();
                result->setProperty("ok", error.isEmpty()); result->setProperty("error", error); result->setProperty("application", "Simplicity GUI shell");
                if (!report_.replaceWithText(juce::JSON::toString(juce::var(result.get())))) error = "Cannot write report";
                setApplicationReturnValue(error.isEmpty() ? 0 : 1); quit();
            });
        }
    }
    void shutdown() override { window_.reset(); }
    void systemRequestedQuit() override { quit(); }
private:
    std::unique_ptr<Window> window_;
    juce::File report_;
};
}
START_JUCE_APPLICATION(SimplicityApplication)
