# wiredpanda: the real Qt Quick-based UI.
# Warning flags, C++ standard, AUTOMOC, etc. are inherited from the root CMakeLists.txt's
# directory scope; nothing needs to be re-set here.

if(EMSCRIPTEN)
    qt_add_executable(wiredpanda Main.cpp)
else()
    add_executable(wiredpanda Main.cpp)
endif()

# qt_add_qml_module's generated QML type registration includes every QML-registered
# (QML_ELEMENT/QML_ANONYMOUS/QML_FOREIGN) header via a bare #if __has_include(<CanvasItem.h>)
# -- which needs the header's own directory on the include path, not just the repo root
# (App/QuickShell/Canvas/CanvasItem.h is normally reached via its full
# "App/QuickShell/Canvas/CanvasItem.h" path against the root -I, which the bare-name form
# can't use). Add each such header's directory explicitly so this keeps working regardless
# of unity-batch luck -- Chrome/ joins Canvas/ here now that QuickAppController is also a
# QML-registered type (its AppControllerForeign QML_FOREIGN wrapper).
target_include_directories(wiredpanda PRIVATE Canvas Chrome Dolphin)

# See the target_include_directories comment above: this target would otherwise rely on the
# bare-name __has_include() resolving by unity-batch accident (whichever translation unit
# happened to include the QML-registered header via its full path landing in the same batch
# as the generated registration file). Disable unity build for this small target so a future
# source-list change can't silently reintroduce that same fragility.
set_target_properties(wiredpanda PROPERTIES UNITY_BUILD OFF)

# Single source of truth for the Quick shell's non-QML sources, consumed both by
# wiredpanda's qt_add_qml_module() below (which needs paths relative to this directory)
# and by test_wiredpanda's target_sources() call further down, so CanvasItem/
# QuickAppController/the Quick MCP handlers get real test coverage -- none of this is part
# of wiredpanda_lib, which test_wiredpanda otherwise links. Absolute paths so the list
# works unmodified from either target's own directory scope.
set(QUICK_SHELL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/CanvasCommands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/CanvasCommands.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/CanvasICRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/CanvasICRegistry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/CanvasItem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/CanvasItem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/SpatialIndex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/SpatialIndex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/TextureAtlas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Canvas/TextureAtlas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/DialogProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/DialogProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickAppController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickAppController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickDialogProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickDialogProvider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickElementEditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickElementEditor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickElementPalette.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickElementPalette.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickExerciseController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickExerciseController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickExerciseEngineBinding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickExportController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickExportController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickICController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickICController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickICPreview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickICPreview.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickMainWindowHost.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickMinimap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickMinimap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickTourController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickTourController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickUpdateController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickUpdateController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickWorkSpace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickWorkSpace.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickWorkspaceManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrome/QuickWorkspaceManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Dolphin/QuickDolphinController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dolphin/QuickDolphinController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Dolphin/QuickDolphinExporter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dolphin/QuickDolphinExporter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Dolphin/QuickDolphinZoom.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Dolphin/QuickDolphinZoom.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Core/QuickMCPProcessor.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Core/QuickMCPProcessor.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickBaseHandler.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickBaseHandler.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickConnectionHandler.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickConnectionHandler.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickElementHandler.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickElementHandler.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickFileHandler.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickFileHandler.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickHistoryHandler.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickHistoryHandler.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickICHandler.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickICHandler.h
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickSimulationHandler.cpp
    ${CMAKE_SOURCE_DIR}/MCP/Server/Handlers/QuickSimulationHandler.h
)

# Exposed to the root CMakeLists.txt's own FUZZERS section (add_subdirectory() scopes are
# child-only by default) -- several libFuzzer harnesses drive CanvasItem/QuickWorkSpace/
# CanvasCommands/CanvasICRegistry directly, the same way test_wiredpanda's own
# target_sources(test_wiredpanda PRIVATE ${QUICK_SHELL_SOURCES}) call below does.
set(QUICK_SHELL_SOURCES ${QUICK_SHELL_SOURCES} PARENT_SCOPE)

qt_add_qml_module(wiredpanda
    URI QuickShell
    VERSION 1.0
    QML_FILES
        ChoiceDialog.qml
        Dolphin/DolphinWindow.qml
        Dolphin/RangeDialog.qml
        ElementEditor.qml
        ElementPalette.qml
        ExerciseOverlay.qml
        ICPreviewPopup.qml
        Main.qml
        Minimap.qml
        PaletteItemDelegate.qml
        QuickFileDialog.qml
        SignalTable.qml
        TextPromptDialog.qml
        ThickScrollBar.qml
        TourOverlay.qml
        TruthTableDialog.qml
        UpdateDialog.qml
    SOURCES
        ${QUICK_SHELL_SOURCES}
)

# Qt's own generated code, not ours to edit: qt_add_qml_module()'s generated
# Registry::lookupCachedUnit() (in <builddir>/App/QuickShell/.rcc/qmlcache/
# wiredpanda_qmlcache_loader.cpp) calls QGlobalStatic::operator(), which trips the same
# recurring GCC 15 -Wnull-dereference false-positive class as the hand-written call sites
# fixed via scoped pragmas elsewhere. Can't add a pragma to code we don't author and that
# gets regenerated every build; scoped to just this one generated file, not project-wide or
# even target-wide.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/.rcc/qmlcache/wiredpanda_qmlcache_loader.cpp
        PROPERTIES COMPILE_OPTIONS "-Wno-null-dereference"
    )
endif()

# test_wiredpanda gets its own private copy of just the 3 QuickDialogProvider-backed
# dialog .qml files, at the exact same qrc:/qt/qml/QuickShell/... path QuickDialogProvider.cpp
# hardcodes -- plain resource embedding (not a second qt_add_qml_module()), since
# test_wiredpanda needs none of the QuickShell module's own QML type registration
# (CanvasItem/AppController/...) for this: none of these 3 files `import QuickShell`, only
# stock QtQuick/QtQuick.Dialogs/QtQuick.Controls/QtQuick.Layouts modules, which resolve via
# Qt's own shared, installation-wide qml/ import path regardless of which executable is
# running. Lets TestQuickDialogProvider construct a real QuickDialogProvider and drive its
# dialogOpened() test hook, instead of only ever testing it through a stub.
qt6_add_resources(test_wiredpanda "quick_dialog_test_qml" PREFIX "/qt/qml/QuickShell"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ChoiceDialog.qml TextPromptDialog.qml QuickFileDialog.qml)

# test_wiredpanda (declared in the root CMakeLists.txt, before this add_subdirectory()
# runs) otherwise only links wiredpanda_lib -- none of the Quick shell classes above are
# reachable from it, since they're compiled directly into the wiredpanda executable
# rather than a shared library. Compiling QUICK_SHELL_SOURCES a second time here (rather than
# splitting them into their own static library) is the simplest fix that doesn't risk
# disturbing wiredpanda's own qt_add_qml_module()-driven QML type registration -- these
# test classes only ever construct CanvasItem/QuickAppController/the Quick MCP handlers
# directly in C++, never load real .qml, so they need none of that registration machinery.
target_sources(test_wiredpanda PRIVATE ${QUICK_SHELL_SOURCES})
target_include_directories(test_wiredpanda PRIVATE Canvas Chrome Dolphin)
target_link_libraries(test_wiredpanda PRIVATE ${QUICK_LIBS} Qt6::PrintSupport)

# test_wiredpanda (unlike wiredpanda, which sets UNITY_BUILD OFF wholesale) keeps
# unity batching for its ~150 pre-existing domain test files -- disabling it project-wide here
# would slow down every routine rebuild for a problem that only affects these Quick shell
# sources. Scope the exclusion to just these files instead: multiple .cpp files in this list
# declare same-named anonymous-namespace helpers (e.g. QuickAppController.cpp and
# QuickElementPalette.cpp both define toQmlUrl()), which a unity batch happening to bundle two
# of them together turns into a real redefinition error.
set_source_files_properties(${QUICK_SHELL_SOURCES}
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

# Copy JSON schema to this target's own output directory -- QuickMCPProcessor resolves it via
# QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json"). Keyed on
# the schema itself (not POST_BUILD): a POST_BUILD command runs only when the target relinks,
# so editing only the schema would leave a stale copy beside the executable and the MCP server
# validating against the old contract.
#
# Copied next to test_wiredpanda too, via its own separate command/output, not merely relying on
# both executables landing in the same flat build directory: that coincidence holds on Linux/
# Windows, but wiredpanda gets MACOSX_BUNDLE further down in this file, which relocates
# $<TARGET_FILE_DIR:wiredpanda> to .../wiredpanda.app/Contents/MacOS -- a different directory
# than test_wiredpanda's plain (non-bundle) one. TestQuickSimulationHandler's own schema-
# validation test resolves the same applicationDirPath()-relative path, so it needs its own copy.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/schema-mcp.stamp
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/MCP/schema-mcp.json
        $<TARGET_FILE_DIR:wiredpanda>/schema-mcp.json
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/schema-mcp.stamp
    DEPENDS ${CMAKE_SOURCE_DIR}/MCP/schema-mcp.json
    COMMENT "Copying JSON schema next to the wiredpanda executable"
    VERBATIM
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/schema-mcp-test.stamp
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/MCP/schema-mcp.json
        $<TARGET_FILE_DIR:test_wiredpanda>/schema-mcp.json
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/schema-mcp-test.stamp
    DEPENDS ${CMAKE_SOURCE_DIR}/MCP/schema-mcp.json
    COMMENT "Copying JSON schema next to the test_wiredpanda executable"
    VERBATIM
)
add_custom_target(copy_mcp_schema ALL DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/schema-mcp.stamp
    ${CMAKE_CURRENT_BINARY_DIR}/schema-mcp-test.stamp
)

# Constructs real domain classes (GraphicElement/Port/Connection/Simulation) and needs their
# real icon/appearance resources -- expected coupling as this UI comes online, not a layering
# violation (wiredpanda_lib itself has no Quick/QML dependency in the other direction).
link_wiredpanda_whole_archive(wiredpanda)
# PrintSupport is for CanvasItem::exportToPdf()'s QPrinter/QPageLayout/QPageSize usage --
# not a Widgets dependency: PrintSupport itself has no QtWidgets coupling for the
# QPrinter/QPageLayout/QPageSize API surface actually used here.
target_link_libraries(wiredpanda PRIVATE ${QUICK_LIBS} Qt6::PrintSupport wiredpanda_resources)

# ========= WINDOWS METADATA ===================================

# Windows VERSIONINFO is configured from the template on every platform so template breakage
# surfaces in any CI run, not just Windows builds. The target's VERSION property does not emit
# VERSIONINFO -- the .rc is what Explorer, installers, and crash-report symbolication read.
configure_file(
    "${CMAKE_SOURCE_DIR}/App/Resources/Windows/wpanda.rc.in"
    "${CMAKE_BINARY_DIR}/wpanda.rc"
    @ONLY
)

if(WIN32)
    set_target_properties(wiredpanda PROPERTIES
        WIN32_EXECUTABLE ON
        RUNTIME_OUTPUT_NAME wiredpanda
        VERSION "${PROJECT_VERSION}"
        DESCRIPTION "wiRedPanda - Logic Circuit Simulator"
        COMPANY_NAME GIBIS-UNIFESP
        FILE_DESCRIPTION "wiRedPanda - Logic Circuit Simulator"
        LEGAL_COPYRIGHT "GIBIS-UNIFESP and the wiRedPanda contributors"
        ORIGINAL_FILENAME wiredpanda.exe
        PRODUCT_NAME "wiRedPanda - Logic Circuit Simulator"
    )

    target_sources(wiredpanda PRIVATE "${CMAKE_BINARY_DIR}/wpanda.rc")

    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt${QT_VERSION_MAJOR}_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET wiredpanda POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --verbose 2 --no-translations --no-system-d3d-compiler --no-opengl-sw --no-compiler-runtime $<TARGET_FILE:wiredpanda>
            COMMENT "Deploying Qt libraries with windeployqt"
            VERBATIM
        )
    else()
        message(WARNING "windeployqt not found - Qt libraries will not be automatically deployed")
    endif()
endif()

if(APPLE)
    # Configure and embed Info.plist (replaces @PROJECT_VERSION@ with actual version). Setting
    # MACOSX_BUNDLE makes install(TARGETS ... BUNDLE DESTINATION .) produce a real .app bundle
    # instead of a bare executable.
    configure_file(
        ${CMAKE_SOURCE_DIR}/App/Resources/macOS/Info.plist.in
        ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
        @ONLY
    )
    set_target_properties(wiredpanda PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
    )
endif()

if(EMSCRIPTEN)
    # Preload example files into Emscripten's virtual filesystem.
    target_link_options(wiredpanda PRIVATE
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/Examples@/Examples"
    )

    # Copy WASM support files and patch the HTML shell.
    set(WASM_RES_DIR "${CMAKE_SOURCE_DIR}/App/Resources/Wasm")
    add_custom_command(TARGET wiredpanda POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${WASM_RES_DIR}/coi-serviceworker.min.js"
            "$<TARGET_FILE_DIR:wiredpanda>/coi-serviceworker.min.js"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${WASM_RES_DIR}/favicon.ico"
            "$<TARGET_FILE_DIR:wiredpanda>/favicon.ico"
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE_DIR:wiredpanda>/wiredpanda.html"
            "$<TARGET_FILE_DIR:wiredpanda>/index.html"
        COMMAND ${CMAKE_COMMAND}
            -DHTML_FILE=$<TARGET_FILE_DIR:wiredpanda>/index.html
            -P "${WASM_RES_DIR}/patch_wasm_html.cmake"
        COMMENT "Copying WASM support files and patching index.html"
        VERBATIM
    )
endif()

# ========= INSTALLATION =============================

install(TARGETS wiredpanda
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install example circuits (found at runtime via InstallRelativePaths::resolve("Examples")).
# Destination is platform-specific: it must match InstallRelativePaths.cpp's own candidate
# list for each OS (Windows only ever looks next to the executable; macOS looks inside the
# bundle's Contents/Resources; Linux looks under share/wiredpanda/ via the AppImage/native-FHS
# candidates).
if(WIN32)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Examples/
        DESTINATION bin/Examples
        COMPONENT examples
    )
elseif(APPLE)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Examples/
        DESTINATION "$<TARGET_BUNDLE_DIR_NAME:wiredpanda>/Contents/Resources/Examples"
        COMPONENT examples
    )
else()
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/Examples/
        DESTINATION share/wiredpanda/Examples
        COMPONENT examples
    )
endif()

# Linux-only desktop integration: .desktop entry, icons, and shared-mime-info definition.
# Meaningless on Windows/macOS (no FreeDesktop menu/icon-theme/mime system there).
if(LINUX)
    configure_file(
        ${CMAKE_SOURCE_DIR}/App/Resources/Freedesktop/wiredpanda.desktop
        ${CMAKE_CURRENT_BINARY_DIR}/wiredpanda.desktop
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wiredpanda.desktop
        DESTINATION share/applications
        COMPONENT desktop
    )

    set(ICON_SIZES 26x26 32x32 48x48 64x64 128x128)
    foreach(SIZE ${ICON_SIZES})
        install(FILES ${CMAKE_SOURCE_DIR}/App/Resources/Assets/Icons/${SIZE}/wpanda.png
            DESTINATION share/icons/hicolor/${SIZE}/apps
            RENAME wpanda.png
            COMPONENT icons
        )
    endforeach()

    install(FILES ${CMAKE_SOURCE_DIR}/App/Resources/Freedesktop/wiredpanda-mime.xml
        DESTINATION share/mime/packages
        RENAME wiredpanda.xml
        COMPONENT mime
    )
endif()
