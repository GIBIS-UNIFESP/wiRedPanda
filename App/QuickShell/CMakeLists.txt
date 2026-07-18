# wiredpanda_quick: the new Qt Quick-based UI. See the "QUICK SHELL" section of the root
# CMakeLists.txt for how this fits into the qtquick-rewrite migration. Warning flags, C++
# standard, AUTOMOC, etc. are inherited from the root CMakeLists.txt's directory scope;
# nothing needs to be re-set here.

set(QUICK_SHELL_SOURCES
    Canvas/CanvasCommands.cpp
    Canvas/CanvasCommands.h
    Canvas/CanvasICRegistry.cpp
    Canvas/CanvasICRegistry.h
    Canvas/CanvasItem.cpp
    Canvas/CanvasItem.h
    Canvas/SpatialIndex.cpp
    Canvas/SpatialIndex.h
    Canvas/TextureAtlas.cpp
    Canvas/TextureAtlas.h
    Chrome/DialogProvider.cpp
    Chrome/DialogProvider.h
    Chrome/QuickAppController.cpp
    Chrome/QuickAppController.h
    Chrome/QuickDialogProvider.cpp
    Chrome/QuickDialogProvider.h
    Chrome/QuickElementEditor.cpp
    Chrome/QuickElementEditor.h
    Chrome/QuickElementPalette.cpp
    Chrome/QuickElementPalette.h
    Chrome/QuickExportController.cpp
    Chrome/QuickExportController.h
    Chrome/QuickMainWindowHost.h
    Chrome/QuickWorkSpace.cpp
    Chrome/QuickWorkSpace.h
    Chrome/QuickWorkspaceManager.cpp
    Chrome/QuickWorkspaceManager.h
    Main.cpp
)

if(EMSCRIPTEN)
    qt_add_executable(wiredpanda_quick ${QUICK_SHELL_SOURCES})
else()
    add_executable(wiredpanda_quick ${QUICK_SHELL_SOURCES})
endif()

# qt_add_qml_module's generated QML type registration includes every QML_ELEMENT-tagged
# header via a bare #if __has_include(<CanvasItem.h>) -- which needs the header's own
# directory on the include path, not just the repo root (App/QuickShell/Canvas/CanvasItem.h
# is normally reached via its full "App/QuickShell/Canvas/CanvasItem.h" path against the
# root -I, which the bare-name form can't use). Add each QML_ELEMENT header's directory
# explicitly so this keeps working regardless of unity-batch luck as QUICK_SHELL_SOURCES grows.
target_include_directories(wiredpanda_quick PRIVATE Canvas)

# See the target_include_directories comment above: this target was relying on the bare-name
# __has_include() resolving by unity-batch accident (whichever translation unit happened to
# include CanvasItem.h via its full path landing in the same batch as the generated
# registration file). Disable unity build for this small target so a future source-list change
# can't silently reintroduce that same fragility.
set_target_properties(wiredpanda_quick PROPERTIES UNITY_BUILD OFF)

qt_add_qml_module(wiredpanda_quick
    URI QuickShell
    VERSION 1.0
    QML_FILES
        ChoiceDialog.qml
        ElementEditor.qml
        ElementPalette.qml
        Main.qml
        QuickFileDialog.qml
        TextPromptDialog.qml
)

# qt_add_qml_module's generated cache-unit lookup table (Registry::lookupCachedUnit(),
# via QHash::value()) trips a false-positive -Werror=null-dereference inside Qt's own
# qhash.h under GCC 15's -O3 inliner -- reproduces identically against Qt 6.9.3 and
# 6.12.0, so it's a GCC/Qt-header interaction, not something either Qt version fixes.
# This is Qt-generated code we don't author and can't add a null-check to, so disable
# just this one warning for just this one generated file rather than project-wide.
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_qmlcache_loader.cpp
    PROPERTIES COMPILE_OPTIONS "-Wno-null-dereference"
)

# CanvasItem constructs real GraphicElement subclasses (And/InputSwitch/Led) and drives them
# through the real Simulation engine (see CanvasItem.h's doc comment) -- it needs the actual
# domain/simulation classes and their embedded SVG icon resources (ElementAppearance::setPixmap()
# loads them at construction time and throws if they're missing), so this target whole-archives
# wiredpanda_lib exactly as wiredpanda/test_wiredpanda already do. This is expected, temporary
# coupling through the migration, not a regression: wiredpanda_lib itself doesn't stop being
# Widgets-coupled until the plan's Phase 8 cutover, and wiredpanda_quick reusing its real,
# unmodified domain classes is precisely the point of Phase 1's "wired to the real Simulation
# engine, not synthetic data" goal.
link_wiredpanda_whole_archive(wiredpanda_quick)
target_link_libraries(wiredpanda_quick PRIVATE ${QUICK_LIBS} wiredpanda_resources)
