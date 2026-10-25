#include <simplicity/workspace_view.h>
#include <simplicity/ui_style.h>
#include <simplicity/browser_panel.h>
#include <simplicity/application_icon.h>
#include "test_support.h"

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI initialiser;
    using namespace simplicity;
    const juce::Font type(ui::font());
    CHECK(type.getTypefaceName() == "Segoe UI"); CHECK(type.getHeight() == 13);
    for (int n=static_cast<int>(ui::Glyph::play); n<=static_cast<int>(ui::Glyph::speaker); ++n) {
        juce::Image symbol(juce::Image::ARGB,32,32,true);
        { juce::Graphics g(symbol); ui::glyph(g,static_cast<ui::Glyph>(n),{4,4,24,24},ui::foreground); }
        int visible=0;
        for(int y=0;y<32;++y)for(int x=0;x<32;++x) {
            const auto pixel=symbol.getPixelAt(x,y);
            if(pixel.getAlpha()>0) { ++visible; CHECK(pixel.getRed()==pixel.getGreen()); CHECK(pixel.getGreen()==pixel.getBlue()); }
        }
        CHECK(visible>15); CHECK(symbol.getPixelAt(0,0).getAlpha()==0);
    }
    const auto icon=juce::ImageFileFormat::loadFrom(applicationIconPng,sizeof(applicationIconPng));
    CHECK(icon.isValid()); CHECK(icon.getWidth()==256); CHECK(icon.getHeight()==256);
    CHECK(icon.getPixelAt(0,0).getAlpha()==0); CHECK(icon.getPixelAt(10,128)==juce::Colour(0xff5b5b5b));
    ui::Look look;
    BrowserPanel::SearchField search;
    search.setLookAndFeel(&look); search.setSize(208,33); search.setFont(ui::font());
    search.setBorder({1,1,1,1}); search.setIndents(31,7); search.setTextToShowWhenEmpty("Search",ui::secondary);
    const auto emptySearch=search.createComponentSnapshot(search.getLocalBounds());
    CHECK(emptySearch.getPixelAt(0,16)==ui::border);
    int placeholderPixels=0;
    for(int y=7;y<27;++y)for(int x=32;x<95;++x)if(emptySearch.getPixelAt(x,y)!=ui::background)++placeholderPixels;
    CHECK(placeholderPixels>10); // Overlay glyph must not swallow native placeholder/outline painting.
    search.setLookAndFeel(nullptr);
    ui::IconButton button("Play",ui::Glyph::play); button.setLookAndFeel(&look); button.setSize(32,32);
    CHECK(button.getName()=="Play"); CHECK(button.getTooltip()=="Play");
    auto normal=button.createComponentSnapshot(button.getLocalBounds());
    CHECK(normal.getPixelAt(2,16)==ui::raised);
    button.setToggleState(true,juce::dontSendNotification);
    auto selected=button.createComponentSnapshot(button.getLocalBounds());
    CHECK(selected.getPixelAt(2,16)==ui::selected);
    button.setLookAndFeel(nullptr);
    simplicity::WorkspaceView view;
    juce::DocumentWindow window("Simplicity GUI tests", juce::Colours::black, 0);
    window.setContentNonOwned(&view, false); window.centreWithSize(1440, 900); window.setVisible(true);
    const auto error = view.runSmokeTest();
    if (error.isNotEmpty()) { std::cerr << error << '\n'; return 1; }
    CHECK(view.workspace().tracks().size() == 4);
    auto image = view.createComponentSnapshot(view.getLocalBounds());
    CHECK(image.isValid()); CHECK(image.getWidth() == 1440); CHECK(image.getHeight() == 900);
    // Real rendered palette / blank-window regression checks (not a mockup).
    CHECK(image.getPixelAt(5, 5) == ui::panel);
    CHECK(image.getPixelAt(5, 65) == ui::panel);
    CHECK(image.getPixelAt(1438, 500) == ui::background || image.getPixelAt(1438, 500) == juce::Colour(0xff1b1b1b));
    // Toolbar/browser must remain neutral even when controls are active. Track/clip colours are allowed below.
    for(int y=0;y<62;++y)for(int x=0;x<image.getWidth();++x) {
        const auto pixel=image.getPixelAt(x,y);
        CHECK(pixel.getRed()==pixel.getGreen()); CHECK(pixel.getGreen()==pixel.getBlue());
    }
    if (argc == 2) {
        const auto output = juce::File::getCurrentWorkingDirectory().getChildFile(juce::String(argv[1]));
        CHECK(output.getParentDirectory().createDirectory().wasOk());
        juce::FileOutputStream stream(output); CHECK(stream.openedOk());
        CHECK(juce::PNGImageFormat().writeImageToStream(image, stream));
    }
    window.clearContentComponent();
}
