# Minimum 3.27: $<COMPILE_ONLY> on the test helper libs below needs it. The
# ...3.31 upper bound keeps policy defaults current on newer CMake, so CMP0156
# (link-line de-duplication) still defaults to NEW on CMake >= 3.29 with no
# explicit cmake_policy(SET) needed.
cmake_minimum_required(VERSION 3.27...3.31)
set(CMAKE_POLICY_VERSION_MINIMUM 3.16) # for nlohmann/json
project(wiredpanda VERSION 5.1.3 LANGUAGES CXX)

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
    find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui Network PrintSupport Multimedia Widgets Svg Test)
else()
    find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Network PrintSupport Multimedia Widgets Svg Test)
endif()

set(QT_VERSION_MAJOR 6)
set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Network Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)

add_compile_definitions(
    APP_VERSION="${PROJECT_VERSION}"
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060200
    QT_MESSAGELOGCONTEXT
)

# MCP Server - disabled on tagged release CI builds, enabled on development branches
if(DEFINED ENV{GITHUB_REF} AND "$ENV{GITHUB_REF}" MATCHES "^refs/tags/")
    # Tagged release CI build — MCP disabled
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
    add_compile_options(/W4 /WX /external:W0 /permissive-)
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
        # info in each .obj instead, making the object cacheable. Release builds
        # have no debug info either way.
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
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
# environments that forbid network access during the build. See issue #404.
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
file(GLOB_RECURSE TS_FILES App/Resources/Translations/wpanda_*.ts)

# Use the COMPONENTS form so cross-compile builds (WASM) consult QT_HOST_PATH
# for host-only tools.  The bare `find_package(Qt6LinguistTools)` form only
# searches CMAKE_PREFIX_PATH, which points at the target Qt — where host
# tools like lrelease aren't shipped.
find_package(Qt6 QUIET COMPONENTS LinguistTools)

qt6_add_resources(RESOURCES_RCC ${RESOURCES})

add_library(wiredpanda_resources OBJECT ${RESOURCES_RCC})
set_target_properties(wiredpanda_resources PROPERTIES UNITY_BUILD OFF)

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

# ========= APP ===================================

# Use qt_add_executable for WASM builds as recommended by Qt docs
if(EMSCRIPTEN)
    qt_add_executable(wiredpanda App/Main.cpp)
else()
    add_executable(wiredpanda App/Main.cpp)
endif()
link_wiredpanda_whole_archive(wiredpanda)
target_link_libraries(wiredpanda PRIVATE wiredpanda_resources)

# Compile .ts → .qm and embed as Qt resources at :/i18n/wpanda_<lang>.qm.
# On Qt 6.9+ MERGE_QT_TRANSLATIONS also bakes qtbase / Widgets / Gui /
# Multimedia / Svg catalogs into the same .qm files so Qt-owned dialogs
# (file picker, message boxes) translate too and RTL flips for ar/he/fa
# via the embedded QT_LAYOUT_DIRECTION string.  No filesystem deployment.
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
endif()

# Preload example files into Emscripten's virtual filesystem
if(EMSCRIPTEN)
    target_link_options(wiredpanda PRIVATE
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/Examples@/Examples"
    )
endif()

# Copy JSON schema to executable directory after build
add_custom_command(TARGET wiredpanda POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/MCP/schema-mcp.json
        $<TARGET_FILE_DIR:wiredpanda>/schema-mcp.json
    COMMENT "Copying JSON schema to executable directory"
    VERBATIM
)

# Copy WASM support files and patch the HTML shell
if(EMSCRIPTEN)
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

# Run windeployqt on Windows to copy Qt dependencies (DLLs, plugins).
# --no-translations: Qt translations are now embedded in the binary via
# qt_add_translations(MERGE_QT_TRANSLATIONS), so no separate translations
# directory is needed in the deploy folder.
if(WIN32)
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

if(ENABLE_PCH)
    target_precompile_headers(wiredpanda REUSE_FROM wiredpanda_lib)
endif()

if(WIN32)
    set(WINDOWS_APP_NAME "wiRedPanda - Logic Circuit Simulator")
    set(RC_FILE "${CMAKE_SOURCE_DIR}/App/Resources/Windows/wpanda.rc")

    set_target_properties(wiredpanda PROPERTIES
        WIN32_EXECUTABLE ON
        RUNTIME_OUTPUT_NAME wiredpanda
        # Enhanced Windows metadata
        VERSION "${PROJECT_VERSION}"
        DESCRIPTION "wiRedPanda - Logic Circuit Simulator"
        COMPANY_NAME GIBIS-UNIFESP
        FILE_DESCRIPTION "wiRedPanda - Logic Circuit Simulator"
        LEGAL_COPYRIGHT "GIBIS-UNIFESP and the wiRedPanda contributors"
        ORIGINAL_FILENAME wiredpanda.exe
        PRODUCT_NAME "wiRedPanda - Logic Circuit Simulator"
    )

    if(EXISTS ${RC_FILE})
        target_sources(wiredpanda PRIVATE ${RC_FILE})
    endif()
endif()

# ========= INSTALLATION =============================

# Install executable
install(TARGETS wiredpanda
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Configure and install .desktop file (replaces @VERSION@ with actual version)
configure_file(
    App/Resources/Freedesktop/wiredpanda.desktop
    ${CMAKE_CURRENT_BINARY_DIR}/wiredpanda.desktop
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/wiredpanda.desktop
    DESTINATION share/applications
    COMPONENT desktop
)

# Install icons in standard FreeDesktop Icon Theme hierarchy
set(ICON_SIZES 26x26 32x32 48x48 64x64 128x128)
foreach(SIZE ${ICON_SIZES})
    install(FILES App/Resources/Assets/Icons/${SIZE}/wpanda.png
        DESTINATION share/icons/hicolor/${SIZE}/apps
        RENAME wpanda.png
        COMPONENT icons
    )
endforeach()

# ========= TESTS ===================================

# Add CTest support
enable_testing()

# Test utilities library (shared helpers for all tests)
add_library(test_utils STATIC ${TEST_UTILS_SOURCES} ${TEST_UTILS_HEADERS})
# Headers only: every test executable whole-archives wiredpanda_lib itself
# (link_wiredpanda_whole_archive), so re-linking it here would put the archive on
# the link line a second time and, with a different link feature, collide with
# WHOLE_ARCHIVE on CMake < 3.29 ("already occurred with feature WHOLE_ARCHIVE").
# $<COMPILE_ONLY> propagates wiredpanda_lib's include/define/feature usage
# requirements without adding it to the link.
target_link_libraries(test_utils PUBLIC "$<COMPILE_ONLY:wiredpanda_lib>")
target_compile_definitions(test_utils PRIVATE CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/Tests)

# Helper library for shared circuit builders (register file, memory tests)
add_library(memory_helpers STATIC ${MEMORY_HELPERS_SOURCES} ${MEMORY_HELPERS_HEADERS})
target_link_libraries(memory_helpers PUBLIC "$<COMPILE_ONLY:wiredpanda_lib>" test_utils)

# ---- Single consolidated test executable ----
# All test classes in one binary; CTest registers per-class entries.
# Usage: ./build/test_wiredpanda ClassName [functionName]
add_executable(test_wiredpanda ${TEST_WIREDPANDA_SOURCES} ${TEST_WIREDPANDA_HEADERS})
link_wiredpanda_whole_archive(test_wiredpanda)
target_link_libraries(test_wiredpanda PRIVATE test_utils wiredpanda_resources memory_helpers)
target_compile_definitions(test_wiredpanda PRIVATE
    CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/Tests
)

if(ENABLE_PCH)
    target_precompile_headers(test_wiredpanda REUSE_FROM wiredpanda_lib)
endif()

if(ENABLE_UNITY)
    message(STATUS "Unity builds enabled for test executable (batch size: ${UNITY_BUILD_BATCH_SIZE})")
    set_target_properties(test_wiredpanda PROPERTIES
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE ${UNITY_BUILD_BATCH_SIZE}
    )
endif()

# ---- libFuzzer harness for the .panda deserializer ----
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

    # Harness 1: raw .panda file (full WorkSpace::load path) + custom mutator
    add_fuzzer_harness(fuzz_deserialize Tests/Fuzz/FuzzDeserialize.cpp)

    # Harness 2: IC blob (IC::deserializeAndLoad directly, skipping outer wrapper)
    add_fuzzer_harness(fuzz_ic_blob Tests/Fuzz/FuzzICBlob.cpp)

    # Harness 3: structure-aware (FuzzedDataProvider builds valid .panda structure)
    add_fuzzer_harness(fuzz_structured Tests/Fuzz/FuzzStructured.cpp)

    # Harness 4: beWavedDolphin waveform format (.dolphin files)
    add_fuzzer_harness(fuzz_waveform Tests/Fuzz/FuzzWaveform.cpp)

    # Harness 5: clipboard paste (BlobRegistry MIME + panda clipboard stream)
    add_fuzzer_harness(fuzz_clipboard Tests/Fuzz/FuzzClipboard.cpp)

    # Harness 6: pre-4.1 flat binary format (GraphicElement::loadOldFormat)
    add_fuzzer_harness(fuzz_old_format Tests/Fuzz/FuzzOldFormat.cpp)

    # Harness 7: ICRegistry blob management (setBlob, initEmbeddedIC, etc.)
    add_fuzzer_harness(fuzz_ic_registry Tests/Fuzz/FuzzICRegistry.cpp)

    # Harness 8: Code generation (Arduino + SystemVerilog exporters, 0% coverage)
    add_fuzzer_harness(fuzz_codegen Tests/Fuzz/FuzzCodeGen.cpp)

    # Harness 9: Save/load round-trip (exercises Serialization::serialize + element save())
    add_fuzzer_harness(fuzz_round_trip Tests/Fuzz/FuzzRoundTrip.cpp)

    # Harness 10: File-backed IC loading (IC::loadFromFile / IC::loadFileDirectly)
    add_fuzzer_harness(fuzz_ic_file Tests/Fuzz/FuzzICFile.cpp)

    # Harness 11: Serialization::copyPandaFile() and readPreamble() (Save As path)
    add_fuzzer_harness(fuzz_copy_panda Tests/Fuzz/FuzzCopyPanda.cpp)

    # Harness 12: Undo/redo command system (Commands.cpp — entirely dark without this)
    add_fuzzer_harness(fuzz_undo Tests/Fuzz/FuzzUndo.cpp)

    message(STATUS "Fuzzer harnesses enabled: fuzz_deserialize fuzz_ic_blob fuzz_structured fuzz_waveform fuzz_clipboard fuzz_old_format fuzz_ic_registry fuzz_codegen fuzz_round_trip fuzz_ic_file fuzz_copy_panda fuzz_undo")
endif()

add_test(NAME TestArduino COMMAND test_wiredpanda TestArduino WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestAudioBox COMMAND test_wiredpanda TestAudioBox WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestBewavedDolphinGui COMMAND test_wiredpanda TestBewavedDolphinGui WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestBuzzer COMMAND test_wiredpanda TestBuzzer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUAlu COMMAND test_wiredpanda TestCPUAlu WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUBranch COMMAND test_wiredpanda TestCPUBranch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUControlUnit COMMAND test_wiredpanda TestCPUControlUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUDecoders COMMAND test_wiredpanda TestCPUDecoders WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUInstructionExecute COMMAND test_wiredpanda TestCPUInstructionExecute WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUInstructionFetch COMMAND test_wiredpanda TestCPUInstructionFetch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUIntegration COMMAND test_wiredpanda TestCPUIntegration WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUMemoryInterface COMMAND test_wiredpanda TestCPUMemoryInterface WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPUProgramCounter COMMAND test_wiredpanda TestCPUProgramCounter WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPURegisterBank COMMAND test_wiredpanda TestCPURegisterBank WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCPURegisters COMMAND test_wiredpanda TestCPURegisters WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestClock COMMAND test_wiredpanda TestClock WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestClocksAdvanced COMMAND test_wiredpanda TestClocksAdvanced WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCommands COMMAND test_wiredpanda TestCommands WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCommon COMMAND test_wiredpanda TestCommon WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestComponents COMMAND test_wiredpanda TestComponents WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnectionSerialization COMMAND test_wiredpanda TestConnectionSerialization WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnectionValidity COMMAND test_wiredpanda TestConnectionValidity WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnections COMMAND test_wiredpanda TestConnections WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDecoder8To256 COMMAND test_wiredpanda TestDecoder8To256 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDialogs COMMAND test_wiredpanda TestDialogs WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDisplays COMMAND test_wiredpanda TestDisplays WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementFactory COMMAND test_wiredpanda TestElementFactory WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementLabel COMMAND test_wiredpanda TestElementLabel WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementLogic COMMAND test_wiredpanda TestElementLogic WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementLogicErrors COMMAND test_wiredpanda TestElementLogicErrors WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementProperties COMMAND test_wiredpanda TestElementProperties WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestEnums COMMAND test_wiredpanda TestEnums WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFeatures COMMAND test_wiredpanda TestFeatures WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFeedback COMMAND test_wiredpanda TestFeedback WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFileDialogProvider COMMAND test_wiredpanda TestFileDialogProvider WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFiles COMMAND test_wiredpanda TestFiles WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGeometry COMMAND test_wiredpanda TestGeometry WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGraphicelementAdvanced COMMAND test_wiredpanda TestGraphicelementAdvanced WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestIC COMMAND test_wiredpanda TestIC WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestICInline COMMAND test_wiredpanda TestICInline WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestIcons COMMAND test_wiredpanda TestIcons WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestInputElements COMMAND test_wiredpanda TestInputElements WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestInputRotary COMMAND test_wiredpanda TestInputRotary WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLED COMMAND test_wiredpanda TestLED WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMainWindowGui COMMAND test_wiredpanda TestMainWindowGui WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1DFlipFlop COMMAND test_wiredpanda TestLevel1DFlipFlop WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1DLatch COMMAND test_wiredpanda TestLevel1DLatch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1JKFlipFlop COMMAND test_wiredpanda TestLevel1JKFlipFlop WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel1SRLatch COMMAND test_wiredpanda TestLevel1SRLatch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2Decoder2To4 COMMAND test_wiredpanda TestLevel2Decoder2To4 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2Decoder3To8 COMMAND test_wiredpanda TestLevel2Decoder3To8 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel2Decoder4To16 COMMAND test_wiredpanda TestLevel2Decoder4To16 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
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
add_test(NAME TestLevel4RippleALU4Bit COMMAND test_wiredpanda TestLevel4RippleALU4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel4RippleAdder4Bit COMMAND test_wiredpanda TestLevel4RippleAdder4Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
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
add_test(NAME TestLevel6RAM256X8 COMMAND test_wiredpanda TestLevel6RAM256X8 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
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
add_test(NAME TestLevel9MultiCycleCPU8Bit COMMAND test_wiredpanda TestLevel9MultiCycleCPU8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLevel9SingleCycleCPU8Bit COMMAND test_wiredpanda TestLevel9SingleCycleCPU8Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLogicGates COMMAND test_wiredpanda TestLogicGates WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMemorySettlingTime COMMAND test_wiredpanda TestMemorySettlingTime WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMUXDEMUXComprehensive COMMAND test_wiredpanda TestMUXDEMUXComprehensive WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMultiplexing COMMAND test_wiredpanda TestMultiplexing WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestNodeLogic COMMAND test_wiredpanda TestNodeLogic WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestNotifyCatch COMMAND test_wiredpanda TestNotifyCatch WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestPriorities COMMAND test_wiredpanda TestPriorities WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestRAMCell1Bit COMMAND test_wiredpanda TestRAMCell1Bit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestRecentFiles COMMAND test_wiredpanda TestRecentFiles WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestScene COMMAND test_wiredpanda TestScene WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSceneConnections COMMAND test_wiredpanda TestSceneConnections WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSceneState COMMAND test_wiredpanda TestSceneState WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSceneUndoredo COMMAND test_wiredpanda TestSceneUndoredo WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSequential COMMAND test_wiredpanda TestSequential WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSequentialLogic COMMAND test_wiredpanda TestSequentialLogic WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSerialization COMMAND test_wiredpanda TestSerialization WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSettings COMMAND test_wiredpanda TestSettings WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSimulation COMMAND test_wiredpanda TestSimulation WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSimulationBlocker COMMAND test_wiredpanda TestSimulationBlocker WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestStatusOps COMMAND test_wiredpanda TestStatusOps WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSystemVerilogExport COMMAND test_wiredpanda TestSystemVerilogExport WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestThemeManager COMMAND test_wiredpanda TestThemeManager WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestTruthTable COMMAND test_wiredpanda TestTruthTable WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestWaveform COMMAND test_wiredpanda TestWaveform WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestWirelessNode COMMAND test_wiredpanda TestWirelessNode WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestWorkspace COMMAND test_wiredpanda TestWorkspace WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestWorkspaceFileops COMMAND test_wiredpanda TestWorkspaceFileops WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestWorkspaceUnit COMMAND test_wiredpanda TestWorkspaceUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# New unit tests from coverage improvement plan
add_test(NAME TestApplication COMMAND test_wiredpanda TestApplication WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestArduinoCodeGenUnit COMMAND test_wiredpanda TestArduinoCodeGenUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCircuitExporter COMMAND test_wiredpanda TestCircuitExporter WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnectionManager COMMAND test_wiredpanda TestConnectionManager WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestCoreSettings COMMAND test_wiredpanda TestCoreSettings WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDemux COMMAND test_wiredpanda TestDemux WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDisplay COMMAND test_wiredpanda TestDisplay WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDisplay7 COMMAND test_wiredpanda TestDisplay7 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDolphinSerializer COMMAND test_wiredpanda TestDolphinSerializer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementContextMenu COMMAND test_wiredpanda TestElementContextMenu WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementEditor COMMAND test_wiredpanda TestElementEditor WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementPalette COMMAND test_wiredpanda TestElementPalette WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestElementTabNavigator COMMAND test_wiredpanda TestElementTabNavigator WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFileDialogProviderUnit COMMAND test_wiredpanda TestFileDialogProviderUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestFileUtils COMMAND test_wiredpanda TestFileUtils WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGraphicElement COMMAND test_wiredpanda TestGraphicElement WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGraphicsView COMMAND test_wiredpanda TestGraphicsView WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestICDropZone COMMAND test_wiredpanda TestICDropZone WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestICRegistry COMMAND test_wiredpanda TestICRegistry WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestICUnit COMMAND test_wiredpanda TestICUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLabeledSlider COMMAND test_wiredpanda TestLabeledSlider WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLanguageManager COMMAND test_wiredpanda TestLanguageManager WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLengthDialog COMMAND test_wiredpanda TestLengthDialog WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestMux COMMAND test_wiredpanda TestMux WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestPropertyShortcutHandler COMMAND test_wiredpanda TestPropertyShortcutHandler WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestConnection COMMAND test_wiredpanda TestConnection WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestPort COMMAND test_wiredpanda TestPort WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestRecentFilesUnit COMMAND test_wiredpanda TestRecentFilesUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSelectionCapabilities COMMAND test_wiredpanda TestSelectionCapabilities WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSimulationUnit COMMAND test_wiredpanda TestSimulationUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestDanglingPointer COMMAND test_wiredpanda TestDanglingPointer WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestSystemVerilogCodeGenUnit COMMAND test_wiredpanda TestSystemVerilogCodeGenUnit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestTrashButton COMMAND test_wiredpanda TestTrashButton WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestUpdateChecker COMMAND test_wiredpanda TestUpdateChecker WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# ============================================================================
# Test Properties: Labels and Timeouts
# ============================================================================
# Labels: unit (fast unit tests), integration (system/IC tests)

# CPU Tests - Integration category (high complexity)
foreach(TEST TestCPUAlu TestCPUBranch TestCPUControlUnit TestCPUDecoders 
        TestCPUInstructionExecute TestCPUInstructionFetch TestCPUIntegration 
        TestCPUMemoryInterface TestCPUProgramCounter TestCPURegisterBank TestCPURegisters)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# Level Tests - Integration category (IC-level hierarchical tests)
foreach(TEST TestLevel1DFlipFlop TestLevel1DLatch TestLevel1JKFlipFlop TestLevel1SRLatch 
        TestLevel2Decoder2To4 TestLevel2Decoder3To8 TestLevel2Decoder4To16 
        TestLevel2FullAdder1Bit TestLevel2HalfAdder TestLevel2MUX2To1 TestLevel2MUX4To1 
        TestLevel2MUX8To1 TestLevel2ParityChecker TestLevel2ParityGenerator 
        TestLevel2PriorityEncoder8To3 TestLevel2PriorityMUX3To1 TestLevel3ALUSelector5Way 
        TestLevel3BCD7SegmentDecoder TestLevel3Comparator4Bit TestLevel3Comparator4BitEquality 
        TestLevel3Register1Bit TestLevel4BinaryCounter4Bit TestLevel4BusMUX4Bit 
        TestLevel4BusMUX8Bit TestLevel4Comparator4Bit TestLevel4JohnsonCounter4Bit 
        TestLevel4RAM4X1 TestLevel4RAM8X1 TestLevel4Register4Bit TestLevel4RingCounter4Bit 
        TestLevel4RippleAdder4Bit TestLevel4RippleALU4Bit TestLevel4ShiftRegisterPISO 
        TestLevel4ShiftRegisterSIPO TestLevel5BarrelRotator TestLevel5BarrelShifter4Bit 
        TestLevel5ClockGatedDecoder TestLevel5Controller4Bit TestLevel5InstructionDecoder4Bit 
        TestLevel5LoadableCounter4Bit TestLevel5ModuloCounter4Bit TestLevel5ProgramCounter4Bit 
        TestLevel5RegisterFile4X4 TestLevel5RegisterFile8X8 TestLevel5UpDownCounter4Bit 
        TestLevel6ALU8Bit TestLevel6ProgramCounter8BitArithmetic TestLevel6RAM256X8 
        TestLevel6Register8Bit TestLevel6RegisterFile8X8 TestLevel6RippleAdder8Bit 
        TestLevel6StackMemoryInterface TestLevel6StackPointer8Bit TestLevel7ALU16Bit 
        TestLevel7CPUProgramCounter8Bit TestLevel7DataForwardingUnit TestLevel7ExecutionDatapath 
        TestLevel7FlagRegister TestLevel7InstructionDecoder8Bit 
        TestLevel7InstructionMemoryInterface TestLevel7InstructionRegister8Bit 
        TestLevel8DecodeStage TestLevel8ExecuteStage TestLevel8FetchStage TestLevel8MemoryStage 
        TestLevel9CPU16BitRISC TestLevel9FetchStage16Bit TestLevel9MultiCycleCPU8Bit 
        TestLevel9SingleCycleCPU8Bit)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# IC-specific deep tests - Integration category
foreach(TEST TestDecoder8To256 TestMemorySettlingTime TestMUXDEMUXComprehensive TestRAMCell1Bit 
        TestSequential)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# File and Simulation Tests - Integration category
foreach(TEST TestArduino TestFeedback TestFiles TestIC TestICInline
        TestSimulation TestSystemVerilogExport TestWorkspace TestWorkspaceFileops)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# Element and Component Unit Tests - Unit category
foreach(TEST TestBewavedDolphinGui TestAudioBox TestBuzzer TestClock TestClocksAdvanced TestComponents TestDisplays
        TestElementLabel TestElementProperties TestFeatures TestGeometry
        TestGraphicelementAdvanced TestIcons TestInputElements TestInputRotary TestLED
        TestLogicGates TestMultiplexing TestSequentialLogic TestTruthTable TestWaveform
        TestWirelessNode)
    set_tests_properties(${TEST} PROPERTIES LABELS "unit")
endforeach()

# Factory and Core Element Tests - Unit category
foreach(TEST TestElementFactory TestElementLogic TestElementLogicErrors TestNodeLogic
        TestStatusOps)
    set_tests_properties(${TEST} PROPERTIES LABELS "unit")
endforeach()

# Connection and Scene Tests - Integration category
foreach(TEST TestConnections TestConnectionSerialization TestConnectionValidity TestScene 
        TestSceneConnections TestSceneState TestSceneUndoredo)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# Tests that touch the system clipboard must not run in parallel (Windows OLE contention).
set_tests_properties(TestScene TestSceneUndoredo PROPERTIES RESOURCE_LOCK clipboard)

# Serialization and Command Tests - Integration category
foreach(TEST TestSerialization TestSimulationBlocker)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# Dangling-pointer regression tests (WIREDPANDA-H2 cluster). These are
# designed to fail on the current tree and to pass once the corresponding
# fixes in App/Element/IC.{h,cpp} and App/Simulation/Simulation.{h,cpp}
# land. See .claude/SENTRY_CRASH_ANALYSIS_WIREDPANDA-H2.md.
set_tests_properties(TestDanglingPointer PROPERTIES LABELS "unit;regression")

# Core Functionality Tests - Unit category
foreach(TEST TestCommands TestCommon TestEnums TestNotifyCatch TestPriorities TestRecentFiles TestSettings
        TestThemeManager)
    set_tests_properties(${TEST} PROPERTIES LABELS "unit")
endforeach()

# GUI tests run sequentially after all other tests complete
set(GUI_TESTS TestDialogs TestFileDialogProvider TestMainWindowGui TestBewavedDolphinGui)
set(ALL_TESTS
    TestCPUAlu TestCPUBranch TestCPUControlUnit TestCPUDecoders
    TestCPUInstructionExecute TestCPUInstructionFetch TestCPUIntegration
    TestCPUMemoryInterface TestCPUProgramCounter TestCPURegisterBank TestCPURegisters
    TestLevel1DFlipFlop TestLevel1DLatch TestLevel1JKFlipFlop TestLevel1SRLatch
    TestLevel2Decoder2To4 TestLevel2Decoder3To8 TestLevel2Decoder4To16
    TestLevel2FullAdder1Bit TestLevel2HalfAdder TestLevel2MUX2To1 TestLevel2MUX4To1
    TestLevel2MUX8To1 TestLevel2ParityChecker TestLevel2ParityGenerator
    TestLevel2PriorityEncoder8To3 TestLevel2PriorityMUX3To1
    TestLevel3ALUSelector5Way TestLevel3BCD7SegmentDecoder TestLevel3Comparator4Bit
    TestLevel3Comparator4BitEquality TestLevel3Register1Bit
    TestLevel4BinaryCounter4Bit TestLevel4BusMUX4Bit TestLevel4BusMUX8Bit
    TestLevel4Comparator4Bit TestLevel4JohnsonCounter4Bit TestLevel4RAM4X1
    TestLevel4RAM8X1 TestLevel4Register4Bit TestLevel4RingCounter4Bit
    TestLevel4RippleALU4Bit TestLevel4RippleAdder4Bit TestLevel4ShiftRegisterPISO
    TestLevel4ShiftRegisterSIPO
    TestLevel5BarrelRotator TestLevel5BarrelShifter4Bit TestLevel5ClockGatedDecoder
    TestLevel5Controller4Bit TestLevel5InstructionDecoder4Bit TestLevel5LoadableCounter4Bit
    TestLevel5ModuloCounter4Bit TestLevel5ProgramCounter4Bit TestLevel5RegisterFile4X4
    TestLevel5RegisterFile8X8 TestLevel6StackMemoryInterface TestLevel5UpDownCounter4Bit
    TestLevel6ALU8Bit TestLevel6ProgramCounter8BitArithmetic TestLevel6RAM256X8
    TestLevel6Register8Bit TestLevel6RegisterFile8X8 TestLevel6RippleAdder8Bit
    TestLevel6StackPointer8Bit
    TestLevel7ALU16Bit TestLevel7CPUProgramCounter8Bit TestLevel7DataForwardingUnit
    TestLevel7ExecutionDatapath TestLevel7FlagRegister TestLevel7InstructionDecoder8Bit
    TestLevel7InstructionMemoryInterface TestLevel7InstructionRegister8Bit
    TestLevel8DecodeStage TestLevel8ExecuteStage TestLevel8FetchStage TestLevel8MemoryStage
    TestLevel9CPU16BitRISC TestLevel9FetchStage16Bit TestLevel9MultiCycleCPU8Bit
    TestLevel9SingleCycleCPU8Bit
    TestDecoder8To256 TestMemorySettlingTime TestRAMCell1Bit TestSequential
    TestMUXDEMUXComprehensive
    TestArduino TestFeedback TestFiles TestIC TestICInline
    TestSimulation TestSystemVerilogExport TestWorkspace TestWorkspaceFileops
    TestIcons TestWaveform TestAudioBox TestBuzzer TestClock TestClocksAdvanced
    TestComponents TestDisplays TestElementLabel TestElementProperties TestFeatures
    TestGeometry TestGraphicelementAdvanced TestInputElements TestInputRotary TestLED
    TestLogicGates TestMultiplexing TestSequentialLogic TestTruthTable TestWirelessNode
    TestElementFactory TestElementLogic TestElementLogicErrors TestNodeLogic TestStatusOps
    TestConnectionSerialization TestConnectionValidity TestConnections TestScene
    TestSceneConnections TestSceneState TestSceneUndoredo
    TestSerialization TestSimulationBlocker
    TestDanglingPointer
    TestCommands TestCommon TestEnums TestNotifyCatch TestPriorities TestRecentFiles TestSettings TestThemeManager)
foreach(TEST ${ALL_TESTS})
    set_tests_properties(${TEST} PROPERTIES FIXTURES_SETUP "non_gui_complete")
endforeach()
foreach(TEST ${GUI_TESTS})
    set_tests_properties(${TEST} PROPERTIES
        LABELS "gui"
        FIXTURES_REQUIRED "non_gui_complete"
        RESOURCE_LOCK "gui_display")
endforeach()

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
