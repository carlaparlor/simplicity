add_executable(simplicity main.cpp)
target_link_libraries(simplicity PRIVATE simplicity_juce_adapter)
simplicity_strict(simplicity)
add_test(NAME juce_bootstrap COMMAND simplicity)
