add_library(simplicity_core STATIC src/core.cpp)
target_include_directories(simplicity_core PUBLIC include)
simplicity_strict(simplicity_core)
