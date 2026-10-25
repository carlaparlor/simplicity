#include <simplicity/workspace_view.h>
#include "test_support.h"

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI initialiser;
    simplicity::WorkspaceView view;
    juce::DocumentWindow window("Simplicity GUI tests", juce::Colours::black, 0);
    window.setContentNonOwned(&view, false); window.centreWithSize(1440, 900); window.setVisible(true);
    const auto error = view.runSmokeTest();
    if (error.isNotEmpty()) { std::cerr << error << '\n'; return 1; }
    CHECK(view.workspace().tracks().size() == 4);
    auto image = view.createComponentSnapshot(view.getLocalBounds());
    CHECK(image.isValid()); CHECK(image.getWidth() == 1440); CHECK(image.getHeight() == 900);
    // Real rendered palette / blank-window regression checks (not a mockup).
    CHECK(image.getPixelAt(5, 5) == juce::Colour(0xff191c1f));
    CHECK(image.getPixelAt(24, 32) == juce::Colour(0xffc1ee85));
    if (argc == 2) {
        const auto output = juce::File::getCurrentWorkingDirectory().getChildFile(juce::String(argv[1]));
        CHECK(output.getParentDirectory().createDirectory().wasOk());
        juce::FileOutputStream stream(output); CHECK(stream.openedOk());
        CHECK(juce::PNGImageFormat().writeImageToStream(image, stream));
    }
    window.clearContentComponent();
}
