# Compile upstream UI modules once, not once per app/test consumer. No juceaide,
# WebView2, browser, CURL, network, or additional installer prerequisite.
juce_add_modules(ALIAS_NAMESPACE juce
  ${juce_SOURCE_DIR}/modules/juce_data_structures
  ${juce_SOURCE_DIR}/modules/juce_events
  ${juce_SOURCE_DIR}/modules/juce_graphics
  ${juce_SOURCE_DIR}/modules/juce_gui_basics)
add_library(simplicity_juce_gui STATIC)
add_library(simplicity::juce_gui ALIAS simplicity_juce_gui)
target_link_libraries(simplicity_juce_gui PRIVATE juce::juce_gui_basics)
target_include_directories(simplicity_juce_gui SYSTEM PUBLIC ${juce_SOURCE_DIR}/modules)
target_compile_definitions(simplicity_juce_gui PUBLIC
  JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 JUCE_STANDALONE_APPLICATION=1
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
  $<$<CONFIG:Debug>:DEBUG=1;_DEBUG=1>)
target_compile_options(simplicity_juce_gui PRIVATE /utf-8)
