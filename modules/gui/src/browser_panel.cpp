#include <simplicity/browser_panel.h>

namespace simplicity {
BrowserPanel::BrowserPanel():juce::FileFilter("Samples and MIDI") {
    setName("Sound browser");
    for(auto* c:std::initializer_list<juce::Component*>{&search,&type,&tree,&path,&details,&documents,&home,&up,&add})addAndMakeVisible(c);
    search.setName("Search library folder");search.setFont(ui::font());search.setTextToShowWhenEmpty("Search folder",ui::secondary);search.setBorder({1,1,1,1});search.setIndents(31,7);search.onTextChange=[this]{refresh();};
    type.setName("Library file type");type.addItem("All sounds",1);type.addItem("Samples",2);type.addItem("MIDI",3);type.setSelectedId(1,juce::dontSendNotification);type.onChange=[this]{refresh();};
    path.setFont(ui::font());path.setName("Current library folder");details.setFont(ui::font());details.setName("Selected file details");details.setColour(juce::Label::textColourId,ui::secondary);
    home.onClick=[this]{setRootDirectory(juce::File::getSpecialLocation(juce::File::userHomeDirectory));};
    documents.onClick=[this]{setRootDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));};
    up.onClick=[this]{setRootDirectory(directory.getDirectory().getParentDirectory());};up.setTooltip("Parent folder");
    add.onClick=[this]{chooser=std::make_unique<juce::FileChooser>("Add a library folder",directory.getDirectory());auto safe=juce::Component::SafePointer<BrowserPanel>(this);chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectDirectories,[safe](const juce::FileChooser& result){if(safe!=nullptr)safe->setRootDirectory(result.getResult());});};
    tree.setName("Library files");tree.setRootItemVisible(false);tree.setDefaultOpenness(false);tree.addListener(this);tree.setColour(juce::TreeView::backgroundColourId,ui::background);
    worker.startThread();setRootDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));
}
BrowserPanel::~BrowserPanel(){tree.removeListener(this);chooser.reset();worker.stopThread(-1);}
bool BrowserPanel::setRootDirectory(const juce::File& root){
    if(!root.isDirectory())return false;
    directory.setDirectory(root,true,true);path.setText(root.getFileName(),juce::dontSendNotification);path.setTooltip(root.getFullPathName());details.setText({},juce::dontSendNotification);return true;
}
void BrowserPanel::setCategory(int value){type.setSelectedId(juce::jlimit(1,3,value),juce::sendNotificationSync);}
void BrowserPanel::setQuery(const juce::String& q){search.setText(q,false);refresh();}
void BrowserPanel::refresh(){{const juce::ScopedLock lock(filterLock);query=search.getText().trim();category=type.getSelectedId();}directory.refresh();}
bool BrowserPanel::acceptsFile(const juce::File& f) const {
    const juce::ScopedLock lock(filterLock);
    const auto ext=f.getFileExtension().toLowerCase();
    const bool midi=ext==".mid" || ext==".midi";
    const bool audio=ext==".wav" || ext==".aif" || ext==".aiff" || ext==".flac" || ext==".ogg" || ext==".mp3";
    return (category==3?midi:category==2?audio:(audio||midi)) && f.getFileName().containsIgnoreCase(query);
}
bool BrowserPanel::isFileSuitable(const juce::File& f) const{return acceptsFile(f);}
bool BrowserPanel::isDirectorySuitable(const juce::File&) const{return true;}
void BrowserPanel::selectionChanged(){const auto f=tree.getSelectedFile();details.setText(f.isDirectory()?"Folder":f.getFileExtension().toUpperCase()+"  /  "+juce::File::descriptionOfSizeInBytes(f.getSize()),juce::dontSendNotification);details.setTooltip(f.getFullPathName());}
void BrowserPanel::fileDoubleClicked(const juce::File& f){if(f.isDirectory())setRootDirectory(f);else details.setText("Import / audition not connected",juce::dontSendNotification);}
void BrowserPanel::resized(){
    const int w=getWidth();search.setBounds(10,40,w-20,32);type.setBounds(10,83,w-20,28);
    documents.setBounds(8,146,106,29);home.setBounds(8,181,106,29);add.setBounds(8,getHeight()-46,106,30);
    path.setBounds(124,116,w-166,28);up.setBounds(w-38,116,26,27);tree.setBounds(120,148,w-128,getHeight()-200);details.setBounds(124,getHeight()-46,w-136,35);
}
void BrowserPanel::paint(juce::Graphics& g){g.fillAll(ui::panel);ui::label(g,"BROWSER",{14,7,getWidth()-28,26});ui::label(g,"PLACES",{14,116,96,25},12,ui::secondary);g.setColour(ui::border);g.drawVerticalLine(117,117,static_cast<float>(getHeight()-12));g.drawVerticalLine(getWidth()-1,0,static_cast<float>(getHeight()));}
}
