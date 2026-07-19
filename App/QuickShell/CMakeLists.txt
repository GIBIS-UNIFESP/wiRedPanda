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
    Chrome/QuickICController.cpp
    Chrome/QuickICController.h
    Chrome/QuickICPreview.cpp
    Chrome/QuickICPreview.h
    Chrome/QuickMainWindowHost.h
    Chrome/QuickMinimap.cpp
    Chrome/QuickMinimap.h
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

# qt_add_qml_module's generated QML type registration includes every QML-registered
# (QML_ELEMENT/QML_ANONYMOUS/QML_FOREIGN) header via a bare #if __has_include(<CanvasItem.h>)
# -- which needs the header's own directory on the include path, not just the repo root
# (App/QuickShell/Canvas/CanvasItem.h is normally reached via its full
# "App/QuickShell/Canvas/CanvasItem.h" path against the root -I, which the bare-name form
# can't use). Add each such header's directory explicitly so this keeps working regardless
# of unity-batch luck as QUICK_SHELL_SOURCES grows -- Chrome/ joined Canvas/ here once
# QuickAppController/QuickElementEditor/QuickElementPalette/QuickWorkSpace also became
# QML-registered types (see those headers).
target_include_directories(wiredpanda_quick PRIVATE Canvas Chrome)

# See the target_include_directories comment above: this target was relying on the bare-name
# __has_include() resolving by unity-batch accident (whichever translation unit happened to
# include CanvasItem.h via its full path landing in the same batch as the generated
# registration file). Disable unity build for this small target so a future source-list change
# can't silently reintroduce that same fragility.
set_target_properties(wiredpanda_quick PROPERTIES UNITY_BUILD OFF)

qt_add_qml_module(wiredpanda_quick
    URI QuickShell
    VERSION 1.0
    # QtQuick.Controls resolves its actual style (Basic/Fusion/Material/...) at runtime by
    # default, which blocks the Qt Quick Compiler from resolving Controls types at all --
    # confirmed via .aotstats: 156/216 bindings across this module fell back to the
    # interpreter, almost entirely "Could not find property/signal" on stock Controls
    # members (MenuItem.triggered, TextField.text, ComboBox.currentIndex, etc.), not our
    # own code. Pinning Fusion here matches App/Main.cpp's own unconditional
    # app.setStyle("Fusion") on the Widgets side -- no user-facing style switcher exists
    # today, so this gives up nothing the app actually offers, and keeps this window
    # visually consistent with the app it's replacing mid-migration.
    IMPORTS
        QtQuick.Controls.Fusion
    QML_FILES
        ChoiceDialog.qml
        ElementEditor.qml
        ElementPalette.qml
        ICPreviewPopup.qml
        Main.qml
        Minimap.qml
        PaletteItemDelegate.qml
        QuickFileDialog.qml
        TextPromptDialog.qml
        TruthTableDialog.qml
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

# Pinning the Controls style above (see the IMPORTS comment) and marking more
# properties FINAL (see each class's Q_PROPERTY block) lets qmlcachegen actually
# compile far more bindings to native C++ instead of falling back to the interpreter --
# the generated code for those newly-compiled bindings trips two more GCC warnings that
# don't reflect anything we authored: string-concatenation wraps an already-QString
# expression in an explicit QString(...) constructor call (-Werror=useless-cast), and
# list<int>.indexOf() (ChoiceDialog.qml's buttonOrder) returns qsizetype narrowed into
# an int without a cast (-Werror=conversion). Same category as the loader suppression
# above: Qt's own generated code, not something we can edit, so disable just these
# warnings on just these generated files. Applied to all 10 QML_FILES' generated .cpp
# outputs (not only whichever ones hit it today) since which specific bindings get
# compiled vs. interpreted is qmlcachegen's own decision and can shift with any future
# QML edit.
set(QUICK_SHELL_QML_CACHEGEN_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_ChoiceDialog_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_ElementEditor_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_ElementPalette_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_ICPreviewPopup_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_Main_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_Minimap_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_PaletteItemDelegate_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_QuickFileDialog_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_TextPromptDialog_qml.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_quick_TruthTableDialog_qml.cpp
)
set_source_files_properties(
    ${QUICK_SHELL_QML_CACHEGEN_SOURCES}
    PROPERTIES COMPILE_OPTIONS "-Wno-useless-cast;-Wno-conversion"
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

# Compile .ts -> .qm and embed as :/i18n/wpanda_<lang>.qm for this target too. qt_add_translations()
# attaches its generated resource directly to whichever target is passed as its own first
# argument -- the root CMakeLists.txt's own call (for the "wiredpanda" executable) doesn't
# propagate here even though wiredpanda_quick links wiredpanda_resources, since translations
# were never routed through that shared library. A second, independent call (same TS_FILES/
# MERGE_QT_TRANSLATIONS/LUPDATE_OPTIONS flags as the root one, all already in scope from the
# parent directory) is the minimal fix -- confirmed necessary, not assumed, by a real
# QuickAppController::languages() diagnostic that found only the hardcoded "en" fallback
# without this. Re-compiles the same .ts sources a second time (real, small, accepted
# duplication) rather than restructuring the existing wiredpanda-target-owned call, which
# project memory feedback_translations_at_release.md flags as infrequently-touched/sensitive.
if(Qt6LinguistTools_FOUND)
    qt_add_translations(wiredpanda_quick
        TS_FILES ${TS_FILES}
        ${_merge_qt_translations}
        ${_lupdate_opts}
    )
endif()
