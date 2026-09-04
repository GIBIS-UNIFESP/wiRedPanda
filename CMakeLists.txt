# Minimum 3.27: $<COMPILE_ONLY> on the test helper libs below needs it. The
# ...3.31 upper bound keeps policy defaults current on newer CMake, so CMP0156
# (link-line de-duplication) still defaults to NEW on CMake >= 3.29 with no
# explicit cmake_policy(SET) needed.
cmake_minimum_required(VERSION 3.27...3.31)
set(CMAKE_POLICY_VERSION_MINIMUM 3.16) # for nlohmann/json
project(wiredpanda VERSION 5.2.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# We use C++20 the language, but not C++20 modules. With the policy max raised to
# 3.31 above (for CMP0156), CMP0155 also defaults to NEW, which turns on per-source
# module-dependency scanning (the ninja "CXX.dd" dyndep step). That scanning is pure
# overhead here and, worse, defeats unity builds — scanned sources are compiled
# individually instead of being batched. Turn it off explicitly: no modules, no scan.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Enable RPATH for development builds with custom Qt installations
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

# Generate compilation database for clazy and clang-tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable conforming preprocessor for MSVC to support __VA_OPT__
if(MSVC)
    add_compile_options(/Zc:preprocessor)
    add_compile_options(/MP)  # Enable parallel compilation
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(EMSCRIPTEN)
    find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui Network PrintSupport Multimedia Widgets Svg Test Quick QuickControls2 QuickDialogs2 Qml)
else()
    find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Network PrintSupport Multimedia Widgets Svg Test Quick QuickControls2 QuickDialogs2 Qml)
endif()

set(QT_VERSION_MAJOR 6)
# wiredpanda_lib's own portable subset -- deliberately no Qt6::Widgets/PrintSupport/Test, so
# nothing that links wiredpanda_lib (including the Quick app) transitively depends on them.
# The wiredpanda/test_wiredpanda targets each link only the additional Qt modules they
# individually need (e.g. PrintSupport for CanvasItem::exportToPdf(), Qt6::Test for the
# test binary).
set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Network Qt6::Multimedia Qt6::Svg)

# Qt Quick/QML/QuickControls2: kept separate from QT_LIBS deliberately -- only the
# wiredpanda target (see the "QUICK SHELL" section below) needs the Quick stack.
# QuickDialogs2 backs Chrome/QuickDialogProvider's confirm/notice dialogs via QtQuick.Dialogs'
# MessageDialog -- the QMessageBox/QInputDialog equivalent for the new UI.
set(QUICK_LIBS Qt6::Quick Qt6::QuickControls2 Qt6::QuickDialogs2 Qt6::Qml)

# QML modules default to the ":/qt/qml/" resource prefix as of this policy (introduced in
# Qt 6.5) -- set it explicitly to silence qt_add_qml_module's dev warning. Guarded per this
# project's usual version-gated-feature pattern since the project's own floor is Qt 6.2.
if(Qt6_VERSION VERSION_GREATER_EQUAL "6.5.0")
    qt_policy(SET QTP0001 NEW)
endif()
if(Qt6_VERSION VERSION_GREATER_EQUAL "6.8.0")
    qt_policy(SET QTP0004 NEW)
endif()

add_compile_definitions(
    APP_VERSION="${PROJECT_VERSION}"
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060200
    QT_MESSAGELOGCONTEXT
)

if(WIN32)
    # windows.h defines min/max function-like macros that break std::min/std::max and
    # std::numeric_limits<>::min/max in any TU it reaches (Main.cpp includes it directly;
    # the Sentry SDK headers pull it into unity batches on release builds).
    add_compile_definitions(NOMINMAX)
endif()

# MCP Server - disabled on tagged release CI builds, enabled on development branches.
# Coverage builds are CI-only instrumentation artifacts that specifically exist to
# measure MCP/Server coverage (see MCP/Client/run_tests.py in coverage.yml), so always
# enable it there regardless of branch — the Coverage workflow only ever runs on master.
if(DEFINED ENV{GITHUB_REF} AND "$ENV{GITHUB_REF}" MATCHES "^refs/tags/")
    # Tagged release CI build — MCP disabled
elseif(ENABLE_COVERAGE)
    add_compile_definitions(ENABLE_MCP_SERVER)
else()
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(NOT GIT_BRANCH MATCHES "^(master|main)$")
            add_compile_definitions(ENABLE_MCP_SERVER)
        endif()
    endif()
endif()

if(MSVC)
    # /wd4702 ("unreachable code"): a known false positive from Qt's own qmetatype.h/qvariant.h
    # template machinery under MSVC's optimizer (/O2, used by the release/debug presets alike) --
    # not our code, and not scopable to one file the way the GCC-15 null-deref false positive
    # (App/QuickShell/CMakeLists.txt) is: it surfaces from different qmlcache-generated
    # translation units depending on which QVariant/QMetaType template instantiations that QML
    # file's generated code happens to touch.
    add_compile_options(/W4 /WX /wd4702 /external:W0 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic -Werror
        -Wconversion -Wsign-conversion
        -Wshadow
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wimplicit-fallthrough
        -Wnull-dereference
        -Wold-style-cast
        -Wformat=2
        -Wcast-align
        -Wdouble-promotion
        -Wundef
        -Wextra-semi
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wsuggest-override
        )
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            add_compile_options(-Wuseless-cast)
        endif()
    endif()
endif()

if(NOT EMSCRIPTEN)
    find_program(MOLD_BIN mold)
    if(MOLD_BIN)
        add_link_options(-fuse-ld=mold)
    endif()
endif()

# Coverage support
if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
    message(STATUS "Code coverage enabled")
else()
    message(STATUS "Code coverage disabled")
endif()

# Unity builds support
option(ENABLE_UNITY "Enable Unity builds for faster compilation" ON)
set(UNITY_BUILD_BATCH_SIZE 8 CACHE STRING "Number of source files to combine in each unity file")
if(ENABLE_UNITY)
    set(CMAKE_UNITY_BUILD ON)
    set(CMAKE_UNITY_BUILD_BATCH_SIZE ${UNITY_BUILD_BATCH_SIZE})
    message(STATUS "Unity builds enabled globally (batch size: ${UNITY_BUILD_BATCH_SIZE})")
else()
    message(STATUS "Unity builds disabled")
endif()

# Sanitizer support
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    if(ENABLE_MEMORY_SANITIZER)
        if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
            add_compile_options(-fsanitize=memory -fPIE)
            add_link_options(-fsanitize=memory -fPIE)
            message(STATUS "Memory Sanitizer enabled")
        else()
            message(WARNING "Memory Sanitizer only supported with Clang")
        endif()
    endif()

    if(ENABLE_ADDRESS_SANITIZER)
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
        message(STATUS "Address Sanitizer enabled")
    endif()

    if(ENABLE_THREAD_SANITIZER)
        add_compile_options(-fsanitize=thread)
        add_link_options(-fsanitize=thread)
        message(STATUS "Thread Sanitizer enabled")
    endif()

    if(ENABLE_UB_SANITIZER)
        # The fuzzer build deliberately uses the conservative UBSan flag set
        # even on Clang.  The strict implicit-conversion / integer checks fire
        # inside Qt headers (e.g. QArrayDataPointer pointer arithmetic during
        # static element registration) before any wiRedPanda code runs and
        # would mask real findings under the libFuzzer driver.
        if(CMAKE_CXX_COMPILER_ID STREQUAL Clang AND NOT ENABLE_FUZZER)
            add_compile_options(-fsanitize=undefined,implicit-integer-truncation,implicit-integer-arithmetic-value-change,implicit-conversion,integer,nullability)
            add_link_options(-fsanitize=undefined,implicit-integer-truncation,implicit-integer-arithmetic-value-change,implicit-conversion,integer,nullability)
        else()
            add_compile_options(-fsanitize=undefined)
            add_link_options(-fsanitize=undefined)
        endif()
        message(STATUS "Undefined Behavior Sanitizer enabled")
    endif()
endif()

find_program(CCACHE_BIN ccache)
if(CCACHE_BIN)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_BIN})
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_BIN})
    if(MSVC)
        # MSVC's default /Zi writes external .pdb files that ccache can't track,
        # so any RelWithDebInfo/Debug compile is uncacheable. /Z7 embeds debug
        # info in each .obj instead, making the object cacheable. Also covers
        # Release now: a real PDB helps Sentry symbolicate release crashes --
        # neither the .pdb itself nor embedding this at compile time bloats
        # the shipped .exe (that needs /DEBUG at link time too, see below).
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo,Release>:Embedded>")
        # Without /DEBUG the linker doesn't emit a .pdb for a plain Release
        # config at all, regardless of the compile-time format above.
        add_link_options("$<$<CONFIG:Release>:/DEBUG>")
    endif()
    message(STATUS "Compiler cache (ccache) enabled")
else()
    message(STATUS "Compiler cache (ccache) not found")
endif()

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

# WebAssembly specific configuration — must come before FetchContent so that
# third-party dependencies are also compiled with the required WASM flags.
if(EMSCRIPTEN)
    # Essential WASM flags
    add_link_options(-sASYNCIFY -Os)

    # Additional optimization flags recommended by Qt docs
    add_compile_options(-Os -flto -pthread)
    add_link_options(-flto)

    message(STATUS "WebAssembly build detected - Adding ASYNCIFY and optimization flags")
endif()

# External dependencies: nlohmann/json and json-schema-validator.
#
# By default these are fetched and built from source via FetchContent. Set
# WIREDPANDA_USE_SYSTEM_JSON=ON to link against the libraries provided by the
# system instead (e.g. Debian's nlohmann-json3-dev and
# nlohmann-json-schema-validator-dev). This is required by distribution build
# environments that forbid network access during the build.
option(WIREDPANDA_USE_SYSTEM_JSON "Use system nlohmann/json and json-schema-validator instead of fetching them" OFF)

if(WIREDPANDA_USE_SYSTEM_JSON)
    find_package(nlohmann_json REQUIRED)
    find_package(nlohmann_json_schema_validator REQUIRED)
    message(STATUS "Using system nlohmann/json and json-schema-validator")
else()
    # FetchContent for external dependencies
    include(FetchContent)

    # nlohmann/json for JSON schema validation
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(json)

    # JSON Schema Validator for nlohmann/json
    FetchContent_Declare(
        json-schema-validator
        GIT_REPOSITORY https://github.com/pboettch/json-schema-validator.git
        GIT_TAG 2.4.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(json-schema-validator)

    # Suppress warnings from json-schema-validator (third-party code)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(nlohmann_json_schema_validator PRIVATE -w)
    endif()
endif()

include(CMakeSources.cmake)

# Find available translation files
file(GLOB_RECURSE TS_FILES CONFIGURE_DEPENDS App/Resources/Translations/wpanda_*.ts)

# Exercise/Tour content and its Weblate-managed translation catalog.
# CONFIGURE_DEPENDS re-runs CMake's configure step when a file is added/removed here
# (best-effort per CMake docs; works with the Ninja generator this project requires), so a
# new .json file is picked up automatically — no CMake or .qrc edits needed.
file(GLOB EXERCISE_JSON_FILES CONFIGURE_DEPENDS App/Resources/Exercises/*.json)
file(GLOB TOUR_JSON_FILES CONFIGURE_DEPENDS App/Resources/Tours/*.json)
file(GLOB EXERCISE_TOUR_CATALOG_JSON_FILES CONFIGURE_DEPENDS App/Resources/Translations/ExerciseTour/*.json)

# Use the COMPONENTS form so cross-compile builds (WASM) consult QT_HOST_PATH
# for host-only tools.  The bare `find_package(Qt6LinguistTools)` form only
# searches CMAKE_PREFIX_PATH, which points at the target Qt — where host
# tools like lrelease aren't shipped.
find_package(Qt6 QUIET COMPONENTS LinguistTools)

# Only used to wire the Exercise/Tour translation-catalog regeneration into
# update_translations below — lupdate can't parse the JSON that catalog is generated from.
find_package(Python3 QUIET COMPONENTS Interpreter)

qt6_add_resources(RESOURCES_RCC ${RESOURCES})

add_library(wiredpanda_resources OBJECT ${RESOURCES_RCC})
set_target_properties(wiredpanda_resources PROPERTIES UNITY_BUILD OFF)

# Embed Exercise/Tour content JSON and their translation catalog directly onto the
# wiredpanda_resources target. Deliberately NOT a .qrc file: rcc can't glob, so a .qrc would
# reintroduce the hardcoded file list this mechanism is meant to eliminate.
qt6_add_resources(wiredpanda_resources "exercises" PREFIX "/Exercises"
    BASE App/Resources/Exercises FILES ${EXERCISE_JSON_FILES})
qt6_add_resources(wiredpanda_resources "tours" PREFIX "/Tours"
    BASE App/Resources/Tours FILES ${TOUR_JSON_FILES})
qt6_add_resources(wiredpanda_resources "exercise-tour-catalog" PREFIX "/i18n/ExerciseTour"
    BASE App/Resources/Translations/ExerciseTour FILES ${EXERCISE_TOUR_CATALOG_JSON_FILES})

# ELF platforms only: wiredpanda_resources is a CMake OBJECT library, which means
# its object files are compiled independently and do not inherit POSITION_INDEPENDENT_CODE
# from the executables that consume them (wiredpanda, test_wiredpanda).
#
# Some Linux distributions (notably Arch Linux) build Qt with -fPIC and mark certain
# internal symbols — such as qt_resourceFeatureZstd in libQt6Core — with ELF protected
# visibility. Protected visibility signals that the DSO owns the symbol and forbids
# consumers from creating a copy of it in the executable's BSS via a copy relocation.
#
# GNU ld 2.44 (released 2024) tightened enforcement of this rule and turned what was
# previously a warning into a hard error. As a result, linking fails with:
#   "copy relocation against non-copyable protected symbol qt_resourceFeatureZstd"
# when qrc_*.cpp files (generated by rcc) are compiled without -fPIC, because the
# compiler emits a direct R_X86_64_PC32 reference that the linker can only satisfy
# through a copy relocation.
#
# The fix is to compile wiredpanda_resources with -fPIC when Qt actually marks the
# symbol as protected, so that external symbol references go through the GOT instead.
# This is intentionally restricted to UNIX/ELF platforms: on Windows (MSVC and MinGW)
# the PE/COFF format has no concept of protected symbols or copy relocations, and on
# macOS the Mach-O dynamic linker uses a different model (two-level namespaces) that
# does not have this problem.
if(UNIX AND NOT APPLE)
    find_program(READELF_EXECUTABLE readelf)
    if(READELF_EXECUTABLE)
        get_target_property(_qt6core_loc Qt6::Core LOCATION)
        execute_process(
            COMMAND ${READELF_EXECUTABLE} -sW ${_qt6core_loc}
            COMMAND grep PROTECTED
            OUTPUT_VARIABLE _qt6core_protected_syms
            ERROR_QUIET
        )
        if(_qt6core_protected_syms)
            set_target_properties(wiredpanda_resources PROPERTIES POSITION_INDEPENDENT_CODE ON)
        endif()
        unset(_qt6core_loc)
        unset(_qt6core_protected_syms)
    endif()
endif()

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

# ========= TRANSLATIONS ===========================

list(LENGTH TS_FILES TS_FILES_COUNT)
message(STATUS "Found ${TS_FILES_COUNT} translation files:")
foreach(ts_file ${TS_FILES})
    get_filename_component(lang ${ts_file} NAME_WE)
    string(REPLACE "wpanda_" "" lang ${lang})
    message(STATUS "  - ${lang}")
endforeach()

# ========= OUTPUT DIRECTORIES ===================================

# Force all output to be in the build root directory, not in Debug/Release subdirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# For multi-config generators (like Visual Studio), override per-config output dirs
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR})
endforeach()

# Link a target against wiredpanda_lib with whole-archive semantics so the
# linker keeps element .o files whose only external symbols are static-
# initialization registrations (ElementInfo<T>::registered).
# The LINK_LIBRARY:WHOLE_ARCHIVE genex (CMake 3.24+) handles the per-platform
# flag (-Wl,--whole-archive on GNU ld, -force_load on Apple ld64, /WHOLEARCHIVE:
# on MSVC link.exe) and, unlike the previous hand-rolled version, doesn't add
# the archive to the link line twice.
function(link_wiredpanda_whole_archive target)
    if(EMSCRIPTEN)
        # CMake's $<LINK_LIBRARY:WHOLE_ARCHIVE> genex is not registered for the
        # CXX link language under emscripten, so the feature-aware form errors
        # out at configure time.  wasm-ld supports the same semantics via
        # `--whole-archive`/`--no-whole-archive` direct flags — use those.
        # target_link_libraries still records the dependency so transitive
        # includes/defines propagate and the build re-runs when the lib changes.
        target_link_options(${target} PRIVATE
            "SHELL:-Wl,--whole-archive $<TARGET_FILE:wiredpanda_lib> -Wl,--no-whole-archive")
        target_link_libraries(${target} PRIVATE wiredpanda_lib)
    else()
        target_link_libraries(${target} PRIVATE
            "$<LINK_LIBRARY:WHOLE_ARCHIVE,wiredpanda_lib>")
    endif()
endfunction()

# ========= LIB ===================================

add_library(wiredpanda_lib STATIC ${SOURCES} ${HEADERS})
# Use the namespaced json-schema-validator target: it is exported by both the
# fetched build and the installed system package, whereas the unqualified
# `nlohmann_json_schema_validator` name only exists in the fetched build.
target_link_libraries(wiredpanda_lib PUBLIC ${QT_LIBS} nlohmann_json::nlohmann_json nlohmann_json_schema_validator::validator)

# Treat external dependencies as system headers to suppress clazy warnings.
# For system libraries the include directories already arrive as SYSTEM via the
# imported targets' INTERFACE_INCLUDE_DIRECTORIES, so this only applies to the
# fetched sources.
if(NOT WIREDPANDA_USE_SYSTEM_JSON)
    target_include_directories(wiredpanda_lib SYSTEM PUBLIC
        ${CMAKE_BINARY_DIR}/_deps/json-src/include
        ${CMAKE_BINARY_DIR}/_deps/json-schema-validator-src/src
    )
endif()

option(ENABLE_PCH "Enable precompiled headers for faster compilation" ON)
if(ENABLE_PCH)
    target_precompile_headers(wiredpanda_lib PRIVATE pch.h)
    message(STATUS "Precompiled headers (PCH) enabled")
else()
    message(STATUS "Precompiled headers (PCH) disabled")
endif()

find_path(SENTRY_INCLUDE_DIR NAMES sentry.h PATHS thirdparty/sentry/include)

if (SENTRY_INCLUDE_DIR)
    message(STATUS "Sentry found: Enabling HAVE_SENTRY")
    add_definitions(-DHAVE_SENTRY)

    include_directories(${SENTRY_INCLUDE_DIR})
    link_directories(thirdparty/sentry/lib)
    target_link_libraries(wiredpanda_lib PUBLIC sentry)
endif()

# ========= QUICK TESTS ===================================
# Portable domain test suite: no Qt Widgets/PrintSupport, no Scene, nothing but
# wiredpanda_lib + Qt Test. This is the project's sole test suite.

enable_testing()

add_executable(test_wiredpanda ${TEST_WIREDPANDA_SOURCES} ${TEST_WIREDPANDA_HEADERS})
link_wiredpanda_whole_archive(test_wiredpanda)
target_link_libraries(test_wiredpanda PRIVATE wiredpanda_resources Qt6::Test)
target_compile_definitions(test_wiredpanda PRIVATE
    CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/Tests
)
if(ENABLE_PCH)
    # Fully portable target, same subset pch.h already covers -- safe to reuse rather than
    # paying for a second precompilation pass.
    target_precompile_headers(test_wiredpanda REUSE_FROM wiredpanda_lib)
endif()

add_test(NAME TestCodeGenUtils COMMAND test_wiredpanda TestCodeGenUtils WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCommon COMMAND test_wiredpanda TestCommon WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestComponents COMMAND test_wiredpanda TestComponents WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnection COMMAND test_wiredpanda TestConnection WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnectionSerialization COMMAND test_wiredpanda TestConnectionSerialization WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnectionValidity COMMAND test_wiredpanda TestConnectionValidity WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnections COMMAND test_wiredpanda TestConnections WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDanglingPointer COMMAND test_wiredpanda TestDanglingPointer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDemux COMMAND test_wiredpanda TestDemux WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDisplay COMMAND test_wiredpanda TestDisplay WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDisplay7 COMMAND test_wiredpanda TestDisplay7 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDisplays COMMAND test_wiredpanda TestDisplays WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDragDropPayload COMMAND test_wiredpanda TestDragDropPayload WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementAppearance COMMAND test_wiredpanda TestElementAppearance WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementFactory COMMAND test_wiredpanda TestElementFactory WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementLogic COMMAND test_wiredpanda TestElementLogic WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementLogicErrors COMMAND test_wiredpanda TestElementLogicErrors WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementProperties COMMAND test_wiredpanda TestElementProperties WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestEnums COMMAND test_wiredpanda TestEnums WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestExerciseTourResources COMMAND test_wiredpanda TestExerciseTourResources WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestExternalFilePath COMMAND test_wiredpanda TestExternalFilePath WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSceneConnections COMMAND test_wiredpanda TestSceneConnections WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFeatures COMMAND test_wiredpanda TestFeatures WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFeedback COMMAND test_wiredpanda TestFeedback WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFiles COMMAND test_wiredpanda TestFiles WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGeometry COMMAND test_wiredpanda TestGeometry WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGraphicElement COMMAND test_wiredpanda TestGraphicElement WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGraphicElementSerializer COMMAND test_wiredpanda TestGraphicElementSerializer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGraphicelementAdvanced COMMAND test_wiredpanda TestGraphicelementAdvanced WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestICInline COMMAND test_wiredpanda TestICInline WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestICUnit COMMAND test_wiredpanda TestICUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestIcons COMMAND test_wiredpanda TestIcons WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestInstallRelativePaths COMMAND test_wiredpanda TestInstallRelativePaths WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestInputElements COMMAND test_wiredpanda TestInputElements WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestInputRotary COMMAND test_wiredpanda TestInputRotary WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLanguageManager COMMAND test_wiredpanda TestLanguageManager WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLogicGates COMMAND test_wiredpanda TestLogicGates WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMUXDEMUXComprehensive COMMAND test_wiredpanda TestMUXDEMUXComprehensive WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMultiplexing COMMAND test_wiredpanda TestMultiplexing WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMux COMMAND test_wiredpanda TestMux WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestNodeLogic COMMAND test_wiredpanda TestNodeLogic WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestNotifyCatch COMMAND test_wiredpanda TestNotifyCatch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSerialization COMMAND test_wiredpanda TestSerialization WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSimulation COMMAND test_wiredpanda TestSimulation WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSimulationBlocker COMMAND test_wiredpanda TestSimulationBlocker WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSimulationUnit COMMAND test_wiredpanda TestSimulationUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestStatusOps COMMAND test_wiredpanda TestStatusOps WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSystemVerilogCodeGenUnit COMMAND test_wiredpanda TestSystemVerilogCodeGenUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSystemVerilogExport COMMAND test_wiredpanda TestSystemVerilogExport WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestTextureAtlas COMMAND test_wiredpanda TestTextureAtlas WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestThemeHandler COMMAND test_wiredpanda TestThemeHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestTourEngine COMMAND test_wiredpanda TestTourEngine WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestTruthTable COMMAND test_wiredpanda TestTruthTable WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestWirelessNode COMMAND test_wiredpanda TestWirelessNode WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUProgramCounter COMMAND test_wiredpanda TestCPUProgramCounter WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUAlu COMMAND test_wiredpanda TestCPUAlu WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUBranch COMMAND test_wiredpanda TestCPUBranch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUControlUnit COMMAND test_wiredpanda TestCPUControlUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUDecoders COMMAND test_wiredpanda TestCPUDecoders WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUInstructionExecute COMMAND test_wiredpanda TestCPUInstructionExecute WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUInstructionFetch COMMAND test_wiredpanda TestCPUInstructionFetch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUIntegration COMMAND test_wiredpanda TestCPUIntegration WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUMemoryInterface COMMAND test_wiredpanda TestCPUMemoryInterface WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPURegisterBank COMMAND test_wiredpanda TestCPURegisterBank WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPURegisters COMMAND test_wiredpanda TestCPURegisters WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDecoder8To256 COMMAND test_wiredpanda TestDecoder8To256 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestICFixtureLayout COMMAND test_wiredpanda TestICFixtureLayout WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1DFlipFlop COMMAND test_wiredpanda TestLevel1DFlipFlop WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1DLatch COMMAND test_wiredpanda TestLevel1DLatch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1JKFlipFlop COMMAND test_wiredpanda TestLevel1JKFlipFlop WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1SRLatch COMMAND test_wiredpanda TestLevel1SRLatch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2Decoder2To4 COMMAND test_wiredpanda TestLevel2Decoder2To4 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2Decoder3To8 COMMAND test_wiredpanda TestLevel2Decoder3To8 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2Decoder4To16 COMMAND test_wiredpanda TestLevel2Decoder4To16 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2Decoder5To32 COMMAND test_wiredpanda TestLevel2Decoder5To32 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2FullAdder1Bit COMMAND test_wiredpanda TestLevel2FullAdder1Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2HalfAdder COMMAND test_wiredpanda TestLevel2HalfAdder WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2MUX2To1 COMMAND test_wiredpanda TestLevel2MUX2To1 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2MUX4To1 COMMAND test_wiredpanda TestLevel2MUX4To1 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2MUX8To1 COMMAND test_wiredpanda TestLevel2MUX8To1 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2ParityChecker COMMAND test_wiredpanda TestLevel2ParityChecker WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2ParityGenerator COMMAND test_wiredpanda TestLevel2ParityGenerator WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2PriorityEncoder8To3 COMMAND test_wiredpanda TestLevel2PriorityEncoder8To3 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2PriorityMUX3To1 COMMAND test_wiredpanda TestLevel2PriorityMUX3To1 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel3ALUSelector5Way COMMAND test_wiredpanda TestLevel3ALUSelector5Way WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel3BCD7SegmentDecoder COMMAND test_wiredpanda TestLevel3BCD7SegmentDecoder WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel3Comparator4Bit COMMAND test_wiredpanda TestLevel3Comparator4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel3Comparator4BitEquality COMMAND test_wiredpanda TestLevel3Comparator4BitEquality WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel3Register1Bit COMMAND test_wiredpanda TestLevel3Register1Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4BinaryCounter4Bit COMMAND test_wiredpanda TestLevel4BinaryCounter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4BusMUX4Bit COMMAND test_wiredpanda TestLevel4BusMUX4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4BusMUX8Bit COMMAND test_wiredpanda TestLevel4BusMUX8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4Comparator4Bit COMMAND test_wiredpanda TestLevel4Comparator4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4JohnsonCounter4Bit COMMAND test_wiredpanda TestLevel4JohnsonCounter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4RAM4X1 COMMAND test_wiredpanda TestLevel4RAM4X1 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4RAM8X1 COMMAND test_wiredpanda TestLevel4RAM8X1 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4Register4Bit COMMAND test_wiredpanda TestLevel4Register4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4RingCounter4Bit COMMAND test_wiredpanda TestLevel4RingCounter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4RippleAdder4Bit COMMAND test_wiredpanda TestLevel4RippleAdder4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4RippleALU4Bit COMMAND test_wiredpanda TestLevel4RippleALU4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4ShiftRegisterPISO COMMAND test_wiredpanda TestLevel4ShiftRegisterPISO WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4ShiftRegisterSIPO COMMAND test_wiredpanda TestLevel4ShiftRegisterSIPO WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5BarrelRotator COMMAND test_wiredpanda TestLevel5BarrelRotator WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5BarrelShifter4Bit COMMAND test_wiredpanda TestLevel5BarrelShifter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5ClockGatedDecoder COMMAND test_wiredpanda TestLevel5ClockGatedDecoder WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5Controller4Bit COMMAND test_wiredpanda TestLevel5Controller4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5InstructionDecoder4Bit COMMAND test_wiredpanda TestLevel5InstructionDecoder4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5LoadableCounter4Bit COMMAND test_wiredpanda TestLevel5LoadableCounter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5ModuloCounter4Bit COMMAND test_wiredpanda TestLevel5ModuloCounter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5ProgramCounter4Bit COMMAND test_wiredpanda TestLevel5ProgramCounter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5RegisterFile4X4 COMMAND test_wiredpanda TestLevel5RegisterFile4X4 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5RegisterFile8X8 COMMAND test_wiredpanda TestLevel5RegisterFile8X8 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel5UpDownCounter4Bit COMMAND test_wiredpanda TestLevel5UpDownCounter4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6ALU8Bit COMMAND test_wiredpanda TestLevel6ALU8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6ProgramCounter8BitArithmetic COMMAND test_wiredpanda TestLevel6ProgramCounter8BitArithmetic WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6RAM8X8 COMMAND test_wiredpanda TestLevel6RAM8X8 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6Register8Bit COMMAND test_wiredpanda TestLevel6Register8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6RegisterFile8X8 COMMAND test_wiredpanda TestLevel6RegisterFile8X8 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6RippleAdder8Bit COMMAND test_wiredpanda TestLevel6RippleAdder8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6StackMemoryInterface COMMAND test_wiredpanda TestLevel6StackMemoryInterface WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel6StackPointer8Bit COMMAND test_wiredpanda TestLevel6StackPointer8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7ALU16Bit COMMAND test_wiredpanda TestLevel7ALU16Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7CPUProgramCounter8Bit COMMAND test_wiredpanda TestLevel7CPUProgramCounter8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7DataForwardingUnit COMMAND test_wiredpanda TestLevel7DataForwardingUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7ExecutionDatapath COMMAND test_wiredpanda TestLevel7ExecutionDatapath WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7FlagRegister COMMAND test_wiredpanda TestLevel7FlagRegister WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7InstructionDecoder8Bit COMMAND test_wiredpanda TestLevel7InstructionDecoder8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7InstructionMemoryInterface COMMAND test_wiredpanda TestLevel7InstructionMemoryInterface WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel7InstructionRegister8Bit COMMAND test_wiredpanda TestLevel7InstructionRegister8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel8DecodeStage COMMAND test_wiredpanda TestLevel8DecodeStage WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel8ExecuteStage COMMAND test_wiredpanda TestLevel8ExecuteStage WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel8FetchStage COMMAND test_wiredpanda TestLevel8FetchStage WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel8MemoryStage COMMAND test_wiredpanda TestLevel8MemoryStage WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel9CPU16BitRISC COMMAND test_wiredpanda TestLevel9CPU16BitRISC WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel9FetchStage16Bit COMMAND test_wiredpanda TestLevel9FetchStage16Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel9MemoryStage16Bit COMMAND test_wiredpanda TestLevel9MemoryStage16Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel9MultiCycleCPU8Bit COMMAND test_wiredpanda TestLevel9MultiCycleCPU8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel9RegisterFile32X16 COMMAND test_wiredpanda TestLevel9RegisterFile32X16 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel9SingleCycleCPU8Bit COMMAND test_wiredpanda TestLevel9SingleCycleCPU8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMemorySettlingTime COMMAND test_wiredpanda TestMemorySettlingTime WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestRAMCell1Bit COMMAND test_wiredpanda TestRAMCell1Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSequential COMMAND test_wiredpanda TestSequential WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestArduino COMMAND test_wiredpanda TestArduino WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestArduinoCodeGenUnit COMMAND test_wiredpanda TestArduinoCodeGenUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestAudioBox COMMAND test_wiredpanda TestAudioBox WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestBuzzer COMMAND test_wiredpanda TestBuzzer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestClock COMMAND test_wiredpanda TestClock WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestClocksAdvanced COMMAND test_wiredpanda TestClocksAdvanced WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestPort COMMAND test_wiredpanda TestPort WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestPriorities COMMAND test_wiredpanda TestPriorities WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSelectionCapabilities COMMAND test_wiredpanda TestSelectionCapabilities WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCanvasCommands COMMAND test_wiredpanda TestCanvasCommands WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCanvasEmbeddedIC COMMAND test_wiredpanda TestCanvasEmbeddedIC WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCanvasInlineIC COMMAND test_wiredpanda TestCanvasInlineIC WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCanvasItemInteraction COMMAND test_wiredpanda TestCanvasItemInteraction WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCanvasItemSmoke COMMAND test_wiredpanda TestCanvasItemSmoke WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickAppController COMMAND test_wiredpanda TestQuickAppController WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickBaseHandler COMMAND test_wiredpanda TestQuickBaseHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickCanvasZoom COMMAND test_wiredpanda TestQuickCanvasZoom WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickConnectionHandler COMMAND test_wiredpanda TestQuickConnectionHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickDialogProvider COMMAND test_wiredpanda TestQuickDialogProvider WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickDolphinController COMMAND test_wiredpanda TestQuickDolphinController WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickElementEditor COMMAND test_wiredpanda TestQuickElementEditor WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickElementHandler COMMAND test_wiredpanda TestQuickElementHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickElementPalette COMMAND test_wiredpanda TestQuickElementPalette WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickExerciseController COMMAND test_wiredpanda TestQuickExerciseController WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickFileHandlerSecurity COMMAND test_wiredpanda TestQuickFileHandlerSecurity WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickHistoryHandler COMMAND test_wiredpanda TestQuickHistoryHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickICHandler COMMAND test_wiredpanda TestQuickICHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickICHandlerSecurity COMMAND test_wiredpanda TestQuickICHandlerSecurity WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickICPreview COMMAND test_wiredpanda TestQuickICPreview WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickMinimap COMMAND test_wiredpanda TestQuickMinimap WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickSimulationHandler COMMAND test_wiredpanda TestQuickSimulationHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickTourController COMMAND test_wiredpanda TestQuickTourController WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickUpdateController COMMAND test_wiredpanda TestQuickUpdateController WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestQuickWorkSpace COMMAND test_wiredpanda TestQuickWorkSpace WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# ========= QUICK SHELL ===================================
# The real Qt Quick UI -- the project's only UI/app target. Own CMakeLists.txt so
# qt_add_qml_module's QML file aliasing is relative to App/QuickShell/ itself, not the repo root.
add_subdirectory(App/QuickShell)

# Compile .ts -> .qm and embed as :/i18n/wpanda_<lang>.qm for wiredpanda.
if(Qt6LinguistTools_FOUND)
    set(_merge_qt_translations)
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.9.0")
        set(_merge_qt_translations MERGE_QT_TRANSLATIONS)
    endif()
    set(_lupdate_opts)
    if(Qt6_VERSION VERSION_GREATER_EQUAL "6.7.0")
        set(_lupdate_opts LUPDATE_OPTIONS
            -tr-function-alias tr+=PANDACEPTION
            -extensions cpp,h,ui
            -no-obsolete
        )
    endif()
    set(QT_NO_MISSING_CATALOG_LANGUAGE_WARNING ON)
    qt_add_translations(wiredpanda
        TS_FILES ${TS_FILES}
        ${_merge_qt_translations}
        ${_lupdate_opts}
    )

    # Regenerate the Weblate-managed Exercise/Tour content catalog (App/Resources/Translations/
    # ExerciseTour/en.json) as part of the same command used for .ts sources, so contributors
    # still only need to remember one target. A separate custom target because lupdate can't
    # parse JSON. update_translations itself doesn't exist yet at this point: since no
    # SOURCE_TARGETS was passed above, qt_add_translations() deferred its real work (including
    # creating update_translations) to the end of this directory's scope via
    # cmake_language(DEFER ...) instead of running it immediately. The add_dependencies() call
    # below must be deferred the same way so it runs after that — deferred calls execute in the
    # order they were scheduled, and this one is scheduled after Qt's.
    if(Python3_FOUND)
        add_custom_target(update_exercise_tour_catalog
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Scripts/generate_exercise_tour_catalog.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Regenerating Weblate Exercise/Tour content translation catalog"
            VERBATIM
        )
        cmake_language(EVAL CODE
            "cmake_language(DEFER DIRECTORY \"${PROJECT_SOURCE_DIR}\" CALL add_dependencies update_translations update_exercise_tour_catalog)"
        )
    endif()
endif()

# ========= FUZZERS ===================================

# ---- libFuzzer harnesses ----
# Clang-only.  Driven by `-fsanitize=fuzzer` plus ASan + UBSan.  Built only
# when ENABLE_FUZZER=ON (see the `fuzzer` CMake preset).  Not registered as a
# CTest target — libFuzzer drives its own input loop.
if(ENABLE_FUZZER)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "ENABLE_FUZZER=ON requires Clang (got ${CMAKE_CXX_COMPILER_ID}). "
                            "Re-run cmake with -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang.")
    endif()

    # Helper macro: apply common fuzzer build settings to a harness target.
    macro(add_fuzzer_harness target source)
        add_executable(${target} ${source})
        link_wiredpanda_whole_archive(${target})
        target_link_libraries(${target} PRIVATE wiredpanda_resources)
        # libFuzzer + ASan + UBSan.  ENABLE_ADDRESS_SANITIZER / ENABLE_UB_SANITIZER
        # apply globally via the project-level options; we only add the fuzzer
        # entry-point flag here.
        target_compile_options(${target} PRIVATE -fsanitize=fuzzer-no-link)
        target_link_options(${target} PRIVATE -fsanitize=fuzzer)
        set_target_properties(${target} PROPERTIES UNITY_BUILD OFF)
    endmacro()

    # Same as add_fuzzer_harness(), plus QUICK_SHELL_SOURCES compiled directly into the
    # target -- mirrors test_wiredpanda's own target_sources(test_wiredpanda PRIVATE
    # ${QUICK_SHELL_SOURCES}) call in App/QuickShell/CMakeLists.txt. For harnesses that drive
    # CanvasItem/QuickWorkSpace/CanvasCommands/CanvasICRegistry directly -- none of that is
    # part of the portable wiredpanda_lib link_wiredpanda_whole_archive() above already pulls
    # in, since it's compiled straight into the wiredpanda/test_wiredpanda executables rather
    # than a shared library (see that file's own doc comment on why).
    macro(add_quick_fuzzer_harness target source)
        add_fuzzer_harness(${target} ${source})
        target_sources(${target} PRIVATE ${QUICK_SHELL_SOURCES})
        target_include_directories(${target} PRIVATE App/QuickShell/Canvas App/QuickShell/Chrome App/QuickShell/Dolphin)
        target_link_libraries(${target} PRIVATE ${QUICK_LIBS} Qt6::PrintSupport)
    endmacro()

    # Harness 1: raw .panda file (full QuickWorkSpace::load path) + custom mutator
    add_quick_fuzzer_harness(fuzz_deserialize Tests/Fuzz/FuzzDeserialize.cpp)

    # Harness 2: IC blob (IC::deserializeAndLoad directly, skipping outer wrapper)
    add_fuzzer_harness(fuzz_ic_blob Tests/Fuzz/FuzzICBlob.cpp)

    # Harness 3: structure-aware (FuzzedDataProvider builds valid .panda structure)
    add_quick_fuzzer_harness(fuzz_structured Tests/Fuzz/FuzzStructured.cpp)

    # Harness 4: beWavedDolphin waveform format (.dolphin files)
    add_fuzzer_harness(fuzz_waveform Tests/Fuzz/FuzzWaveform.cpp)

    # Harness 5: clipboard paste (BlobRegistry MIME + panda clipboard stream)
    add_quick_fuzzer_harness(fuzz_clipboard Tests/Fuzz/FuzzClipboard.cpp)

    # Harness 6: pre-4.1 flat binary format (GraphicElement::loadOldFormat)
    add_quick_fuzzer_harness(fuzz_old_format Tests/Fuzz/FuzzOldFormat.cpp)

    # Harness 7: CanvasICRegistry blob management (setBlob, initEmbeddedIC, etc.)
    add_quick_fuzzer_harness(fuzz_ic_registry Tests/Fuzz/FuzzICRegistry.cpp)

    # Harness 8: Code generation (Arduino + SystemVerilog exporters, 0% coverage)
    add_quick_fuzzer_harness(fuzz_codegen Tests/Fuzz/FuzzCodeGen.cpp)

    # Harness 9: Save/load round-trip (exercises Serialization::serialize + element save())
    add_quick_fuzzer_harness(fuzz_round_trip Tests/Fuzz/FuzzRoundTrip.cpp)

    # Harness 10: File-backed IC loading (IC::loadFromFile / IC::loadFileDirectly)
    add_quick_fuzzer_harness(fuzz_ic_file Tests/Fuzz/FuzzICFile.cpp)

    # Harness 11: Serialization::copyPandaFile() and readPreamble() (Save As path)
    add_fuzzer_harness(fuzz_copy_panda Tests/Fuzz/FuzzCopyPanda.cpp)

    # Harness 12: Undo/redo command system (CanvasCommands.cpp — entirely dark without this)
    add_quick_fuzzer_harness(fuzz_undo Tests/Fuzz/FuzzUndo.cpp)

    message(STATUS "Fuzzer harnesses enabled: fuzz_deserialize fuzz_ic_blob fuzz_structured fuzz_waveform fuzz_clipboard fuzz_old_format fuzz_ic_registry fuzz_codegen fuzz_round_trip fuzz_ic_file fuzz_copy_panda fuzz_undo")
endif()

# ========= DOCUMENTATION ===================================

find_package(Doxygen OPTIONAL_COMPONENTS dot QUIET)
if(DOXYGEN_FOUND)
    if(Doxygen_dot_FOUND)
        set(DOXYGEN_HAVE_DOT YES)
        get_filename_component(DOXYGEN_DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}" DIRECTORY)
    else()
        set(DOXYGEN_HAVE_DOT NO)
        set(DOXYGEN_DOT_PATH "")
    endif()

    configure_file(
        ${CMAKE_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_BINARY_DIR}/docs/Doxyfile
        @ONLY
    )

    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    message(STATUS "Doxygen found: 'cmake --build --preset debug --target doxygen' to generate docs")
    message(STATUS "  Output: ${CMAKE_BINARY_DIR}/docs/html/index.html")
else()
    message(STATUS "Doxygen not found: documentation target unavailable (install doxygen)")
endif()
