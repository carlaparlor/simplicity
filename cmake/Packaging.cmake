# Windows-only distribution surface. Building never publishes a public release.
install(TARGETS simplicity RUNTIME DESTINATION .)
install(FILES "${PROJECT_SOURCE_DIR}/packaging/windows/README.txt" DESTINATION .)
install(FILES "${PROJECT_SOURCE_DIR}/docs/dependencies.md" DESTINATION notices)
install(FILES "${juce_SOURCE_DIR}/LICENSE.md" DESTINATION notices RENAME JUCE-LICENSE.md)
