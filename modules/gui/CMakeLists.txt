add_library(simplicity_gui STATIC src/browser_panel.cpp src/ui_assets.cpp src/ui_style.cpp src/workspace_view.cpp)
target_include_directories(simplicity_gui PUBLIC include)
target_link_libraries(simplicity_gui PUBLIC simplicity_shell simplicity::juce_gui)
simplicity_strict(simplicity_gui)
