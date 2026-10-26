#include <simplicity/workspace_view.h>
#include <simplicity/ui_style.h>
#include <simplicity/browser_panel.h>
#include <simplicity/pattern_panel.h>
#include <simplicity/application_icon.h>
#include <simplicity/zoom_controls.h>
#include <limits>
#include "test_support.h"

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI initialiser;
    using namespace simplicity;
    ui::Look look;
    const juce::Font type(ui::font());
    CHECK(type.getTypefaceName() == "RetroByte"); CHECK(type.getHeight() == 14);
    for (int n=static_cast<int>(ui::Glyph::play); n<=static_cast<int>(ui::Glyph::maximize); ++n) {
        juce::Image symbol(juce::Image::ARGB,32,32,true);
        { juce::Graphics g(symbol); ui::glyph(g,static_cast<ui::Glyph>(n),{4,4,24,24},ui::foreground); }
        int visible=0;
        for(int y=0;y<32;++y)for(int x=0;x<32;++x) {
            const auto pixel=symbol.getPixelAt(x,y);
            if(pixel.getAlpha()>0) { ++visible; CHECK(pixel.getRed()==pixel.getGreen()); CHECK(pixel.getGreen()==pixel.getBlue()); }
        }
        CHECK(visible>15); CHECK(symbol.getPixelAt(0,0).getAlpha()==0);
    }
    { juce::Image padded(juce::Image::ARGB,32,32,true);
      { juce::Graphics g(padded); ui::glyph(g,ui::Glyph::minus,{4,4,24,24},ui::foreground); }
      // End the Graphics context before readback: Direct2D batches until EndDraw.
      if(padded.getPixelAt(16,16).getAlpha()<=200) {
          int minX=32,minY=32,maxX=-1,maxY=-1;
          for(int y=0;y<32;++y)for(int x=0;x<32;++x)if(padded.getPixelAt(x,y).getAlpha()>0) {
              minX=std::min(minX,x); minY=std::min(minY,y); maxX=std::max(maxX,x); maxY=std::max(maxY,y);
          }
          std::cerr<<"minus readback alpha="<<static_cast<int>(padded.getPixelAt(16,16).getAlpha())<<" bounds="<<minX<<","<<minY<<","<<maxX<<","<<maxY<<"\n";
      }
      CHECK(padded.getPixelAt(5,16).getAlpha()==0); // Original 24x24 viewBox padding, not ink-bounds fitting.
      CHECK(padded.getPixelAt(16,14).getAlpha()==0); // Upstream 2-unit stroke, not an expanded thick stroke.
      CHECK(padded.getPixelAt(16,16).getAlpha()>200); }
    const auto icon=juce::ImageFileFormat::loadFrom(applicationIconPng,sizeof(applicationIconPng));
    CHECK(icon.isValid()); CHECK(icon.getWidth()==256); CHECK(icon.getHeight()==256);
    CHECK(icon.getPixelAt(0,0).getAlpha()==0); CHECK(icon.getPixelAt(10,128)==juce::Colour(0xff181818));
    CHECK(look.getTypefaceForFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(),13,juce::Font::plain)))->getName()=="RetroByte");
    // Compare the icon's opaque mask with the actual embedded font's s path.
    juce::GlyphArrangement letter; letter.addLineOfText(juce::Font(ui::font(100)),"s",0,100);
    CHECK(letter.getNumGlyphs()==1);
    juce::Path letterPath; letter.getGlyph(0).createPath(letterPath);
    CHECK(!letterPath.isEmpty());
    const float h=256.0F*0.62F, w=h*6.0F/7.0F;
    letterPath.applyTransform(letterPath.getTransformToScaleToFit((256-w)/2,(256-h)/2,w,h,true));
    int opaqueInk=0;
    for(int y=0;y<256;++y)for(int x=0;x<256;++x) {
        const auto pixel=icon.getPixelAt(x,y);
        const bool inside=letterPath.contains(static_cast<float>(x)+0.5F,static_cast<float>(y)+0.5F);
        if(pixel.getRed()==238) { ++opaqueInk; CHECK(inside); }
        if(pixel.getRed()==24) CHECK(!inside);
    }
    CHECK(opaqueInk>1000);
    // Tinting a cached Lucide icon must not retain the previous track colour.
    for(auto tint:{juce::Colours::red,ui::foreground}) {
        juce::Image image(juce::Image::ARGB,32,32,true);
        { juce::Graphics g(image); ui::glyph(g,ui::Glyph::search,{4,4,24,24},tint); }
        int opaque=0;
        for(int y=0;y<32;++y)for(int x=0;x<32;++x)if(image.getPixelAt(x,y).getAlpha()>200) {
            ++opaque;
            if(tint==ui::foreground) CHECK(image.getPixelAt(x,y).getRed()==image.getPixelAt(x,y).getBlue());
            else CHECK(image.getPixelAt(x,y).getRed()>200 && image.getPixelAt(x,y).getBlue()==0);
        }
        CHECK(opaque>10); // Do not let an empty/unflushed image make the tint test vacuous.
    }
    { juce::Image empty(juce::Image::ARGB,32,32,true);
      { juce::Graphics g(empty);
        ui::glyph(g,ui::Glyph::none,{0,0,32,32},ui::foreground);
        ui::glyph(g,static_cast<ui::Glyph>(999),{0,0,32,32},ui::foreground); }
      for(int y=0;y<32;++y)for(int x=0;x<32;++x)CHECK(empty.getPixelAt(x,y).getAlpha()==0); }
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
    CHECK(normal.getPixelAt(16,2).getRed()>normal.getPixelAt(16,29).getRed());
    CHECK(normal.getPixelAt(2,16).getRed()==normal.getPixelAt(2,16).getBlue());
    button.setToggleState(true,juce::dontSendNotification);
    auto selected=button.createComponentSnapshot(button.getLocalBounds());
    CHECK(selected.getPixelAt(2,16).getRed()>normal.getPixelAt(2,16).getRed());
    button.setLookAndFeel(nullptr);
    // Previews must be a function of stored notes, including repeats/cropped tails.
    Pattern preview;preview.length=384;preview.notes.push_back({0,96,60,3,100});
    juce::Image notes(juce::Image::ARGB,120,48,true);
    {juce::Graphics g(notes);drawPatternNotes(g,preview,{0,0,120,48},0,8,ui::foreground);}
    CHECK(notes.getPixelAt(2,41).getAlpha()>200);CHECK(notes.getPixelAt(62,41).getAlpha()>200);CHECK(notes.getPixelAt(32,41).getAlpha()==0);
    preview.notes.clear();juce::Image blank(juce::Image::ARGB,120,48,true);
    {juce::Graphics g(blank);drawPatternNotes(g,preview,{0,0,120,48},0,8,ui::foreground);}
    for(int y=0;y<48;++y)for(int x=0;x<120;++x)CHECK(blank.getPixelAt(x,y).getAlpha()==0);
    simplicity::WorkspaceView view;
    juce::DocumentWindow window("Simplicity GUI tests", juce::Colours::black, 0);
    window.setContentNonOwned(&view, false); window.centreWithSize(1440, 900); window.setVisible(true);
    // Exercise real handle edges, pan, bounds, callbacks and keyboard navigation.
    TimelineZoomBar bar;view.addAndMakeVisible(bar);bar.setBounds(0,0,400,18);int rangeChanges=0;bar.changed=[&]{++rangeChanges;};
    const auto event=[](juce::Component& target,juce::Point<float> down,juce::Point<float> at,int mods=juce::ModifierKeys::leftButtonModifier){return juce::MouseEvent(juce::Desktop::getInstance().getMainMouseSource(),at,juce::ModifierKeys(mods),1.0F,0,0,0,0,&target,&target,juce::Time::getCurrentTime(),down,juce::Time::getCurrentTime(),1,true);};
    bar.setRange(0.25,0.5);CHECK(rangeChanges==0);bar.mouseDown(event(bar,{299,8},{299,8}));bar.mouseDrag(event(bar,{299,8},{199,8}));CHECK(std::abs(bar.start()-0.25)<1e-6);CHECK(std::abs(bar.span()-0.25)<1e-6);
    bar.setRange(0.25,0.5);bar.mouseDown(event(bar,{101,8},{101,8}));bar.mouseDrag(event(bar,{101,8},{201,8}));CHECK(std::abs(bar.start()-0.5)<1e-6);CHECK(std::abs(bar.span()-0.25)<1e-6);
    bar.setRange(0.25,0.25);bar.mouseDown(event(bar,{150,8},{150,8}));bar.mouseDrag(event(bar,{150,8},{230,8}));CHECK(std::abs(bar.start()-0.45)<1e-6);CHECK(rangeChanges==3);
    bar.setRange(-1,2);CHECK(bar.start()==0 && bar.span()==1);bar.setRange(1,0);CHECK(bar.start()==1-TimelineZoomBar::minimumSpan);CHECK(bar.span()==TimelineZoomBar::minimumSpan);
    bar.setRange(std::numeric_limits<double>::quiet_NaN(),0.5);CHECK(bar.span()==TimelineZoomBar::minimumSpan);bar.zoom(0,0.5);bar.zoom(-1,0.5);bar.zoom(std::numeric_limits<double>::infinity(),0.5);CHECK(bar.span()==TimelineZoomBar::minimumSpan);
    bar.mouseDoubleClick(event(bar,{0,0},{0,0}));CHECK(bar.start()==0 && bar.span()==1);CHECK(bar.keyPressed(juce::KeyPress(juce::KeyPress::pageUpKey)));CHECK(bar.span()<1);CHECK(!bar.keyPressed(juce::KeyPress('Q')));
    const double before=bar.start();CHECK(bar.keyPressed(juce::KeyPress(juce::KeyPress::rightKey)));CHECK(bar.start()>before);CHECK(bar.keyPressed(juce::KeyPress(juce::KeyPress::leftKey)));
    VerticalZoomGrip grip;view.addAndMakeVisible(grip);grip.setBounds(400,0,16,18);double factor=0;int resets=0;grip.scale=[&](double value){factor=value;};grip.reset=[&]{++resets;};grip.mouseDown(event(grip,{8,8},{8,8}));grip.mouseDrag(event(grip,{8,8},{8,-88}));CHECK(std::abs(factor-2)<1e-6);grip.mouseDoubleClick(event(grip,{8,8},{8,8}));CHECK(resets==1);CHECK(grip.keyPressed(juce::KeyPress(juce::KeyPress::downKey)));CHECK(factor<1);
    view.removeChildComponent(&bar);view.removeChildComponent(&grip);view.grabKeyboardFocus();
    // Two columns, an odd final row and the same actual clip raster as the playlist.
    Workspace sources;PatternPanel picker(sources);picker.setLookAndFeel(&look);picker.setSize(228,400);picker.refresh();
    const auto a=picker.cardBounds(0),b=picker.cardBounds(1),c=picker.cardBounds(2);CHECK(a.getY()==b.getY());CHECK(!a.intersects(b));CHECK(c.getX()==a.getX());CHECK(c.getY()>a.getBottom());CHECK(picker.cardBounds(-1).isEmpty());CHECK(picker.cardBounds(4).isEmpty());
    std::function<juce::Component*(juce::Component&)> findCard=[&](juce::Component& component)->juce::Component*{if(component.getComponentID()=="pattern-card-1")return &component;for(int n=0;n<component.getNumChildComponents();++n)if(auto* found=findCard(*component.getChildComponent(n)))return found;return nullptr;};
    auto* card=findCard(picker);CHECK(card!=nullptr);const auto cardImage=card->createComponentSnapshot(card->getLocalBounds());juce::Image expected(juce::Image::ARGB,a.getWidth(),a.getHeight(),true);{juce::Graphics g(expected);drawPatternClip(g,*sources.pattern(1),card->getLocalBounds().toFloat().reduced(3),0,4,true);}
    for(int y=5;y<a.getHeight()-5;++y)for(int x=5;x<a.getWidth()-5;++x)CHECK(cardImage.getPixelAt(x,y)==expected.getPixelAt(x,y));
    sources.newSession();picker.refresh();CHECK(picker.cardBounds(0).isEmpty());for(int n=0;n<3;++n)CHECK(sources.addPattern("Card"));picker.refresh();CHECK(!picker.cardBounds(2).isEmpty());CHECK(picker.cardBounds(3).isEmpty());
    for(int width:{190,228,300}){picker.setSize(width,300);CHECK(!picker.cardBounds(0).intersects(picker.cardBounds(1)));CHECK(picker.cardBounds(1).getRight()<width);}
    picker.setLookAndFeel(nullptr);
    const auto error = view.runSmokeTest();
    if (error.isNotEmpty()) { std::cerr << error << '\n'; return 1; }
    CHECK(view.workspace().tracks().size() == 4);
    auto image = view.createComponentSnapshot(view.getLocalBounds());
    CHECK(image.isValid()); CHECK(image.getWidth() == 1440); CHECK(image.getHeight() == 900);
    // Real rendered palette / blank-window regression checks (not a mockup).
    CHECK(image.getPixelAt(5, 5) == ui::panel);
    CHECK(image.getPixelAt(5, 65) == ui::panel);
    CHECK(image.getPixelAt(1438, 500) == ui::panel); // Patterns is explicitly the rightmost dock.
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
