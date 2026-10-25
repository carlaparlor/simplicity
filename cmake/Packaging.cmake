# Windows-only distribution surface. Building never publishes a public release.
install(TARGETS simplicity RUNTIME DESTINATION .)
install(FILES "${PROJECT_SOURCE_DIR}/packaging/windows/README.txt" DESTINATION .)
install(FILES "${PROJECT_SOURCE_DIR}/docs/dependencies.md" DESTINATION notices)
install(FILES "${juce_SOURCE_DIR}/LICENSE.md" DESTINATION notices RENAME JUCE-LICENSE.md)
install(FILES "${PROJECT_SOURCE_DIR}/modules/gui/assets/lucide/LICENSE" DESTINATION notices RENAME LUCIDE-LICENSE.txt)
install(FILES "${PROJECT_SOURCE_DIR}/modules/gui/assets/retrobyte/NOTICE.txt" DESTINATION notices RENAME RETROBYTE-NOTICE.txt)
