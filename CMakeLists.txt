cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_VERSION_MINIMUM 3.16) # for nlohmann/json
project(wiredpanda VERSION 4.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    add_compile_options(/W4 /external:W0 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
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
            -Wuseless-cast
            -Wsuggest-override
        )
    endif()
endif()

find_program(MOLD_BIN mold)
if(MOLD_BIN)
    add_link_options(-fuse-ld=mold)
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
        if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
            add_compile_options(-fsanitize=undefined,implicit-integer-truncation,implicit-integer-arithmetic-value-change,implicit-conversion,integer,nullability)
            add_link_options(-fsanitize=undefined,implicit-integer-truncation,implicit-integer-arithmetic-value-change,implicit-conversion,integer,nullability)
        else()
            add_compile_options(-fsanitize=undefined)
            add_link_options(-fsanitize=undefined)
        endif()
        message(STATUS "Undefined Behavior Sanitizer enabled")
    endif()
endif()

if(NOT MSVC)
    find_program(CCACHE_BIN ccache)
    if(CCACHE_BIN)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_BIN})
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_BIN})
        message(STATUS "Compiler cache (ccache) enabled")
    else()
        message(STATUS "Compiler cache (ccache) not found")
    endif()
endif()

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

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
    GIT_TAG 2.3.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json-schema-validator)

# Suppress warnings from json-schema-validator (third-party code)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(nlohmann_json_schema_validator PRIVATE -w)
endif()

# WebAssembly specific configuration
if(EMSCRIPTEN)
    # Essential WASM flags
    add_link_options(-sASYNCIFY -Os)

    # Additional optimization flags recommended by Qt docs
    add_compile_options(-Os -flto)
    add_link_options(-flto)

    message(STATUS "WebAssembly build detected - Adding ASYNCIFY and optimization flags")
endif()

include(CMakeSources.cmake)

# Find available translation files
file(GLOB_RECURSE TS_FILES App/Resources/Translations/wpanda_*.ts)

# Initialize QM_FILES list
set(QM_FILES)

# Create QM files in source directory so QRC can find them
find_package(Qt6LinguistTools QUIET)
if(Qt6LinguistTools_FOUND)
    foreach(ts_file ${TS_FILES})
        get_filename_component(qm_file ${ts_file} NAME_WE)
        get_filename_component(ts_dir ${ts_file} DIRECTORY)
        set(qm_path ${ts_dir}/${qm_file}.qm)
        add_custom_command(
            OUTPUT ${qm_path}
            COMMAND Qt6::lrelease ${ts_file} -qm ${qm_path}
            DEPENDS ${ts_file}
            COMMENT "Compiling ${qm_file}.qm"
            VERBATIM
        )
        list(APPEND QM_FILES ${qm_path})
    endforeach()
endif()
qt6_add_resources(RESOURCES_RCC ${RESOURCES})

add_library(wiredpanda_resources OBJECT ${RESOURCES_RCC})
set_target_properties(wiredpanda_resources PROPERTIES UNITY_BUILD OFF)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

# ========= TRANSLATIONS ===========================

# Find available translation files
file(GLOB_RECURSE TS_FILES App/Resources/Translations/wpanda_*.ts)

# Custom targets for translation management
if(Qt6LinguistTools_FOUND)
    # Custom target to update translation source files
    add_custom_target(lupdate
        COMMAND Qt6::lupdate -tr-function-alias tr+=PANDACEPTION -extensions cpp,h,ui -I ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/App -ts ${TS_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Updating translation source files"
    )

    # Custom target to compile translation files
    add_custom_target(lrelease
        DEPENDS ${QM_FILES}
        COMMENT "Compiling translation files"
    )

    # Make sure resources depend on QM files
    add_custom_target(compile_translations ALL DEPENDS ${QM_FILES})
endif()

# CMake script to generate translations.qrc natively
set(QRC_GENERATOR_SCRIPT "${CMAKE_BINARY_DIR}/generate_translations_qrc.cmake")

# Write the QRC generator script
file(WRITE ${QRC_GENERATOR_SCRIPT} "
# CMake script to generate translations.qrc
set(QRC_FILE \"${CMAKE_SOURCE_DIR}/App/Resources/Translations/Translations.qrc\")

# Get list of .qm files in translations directory
file(GLOB QM_FILES \"${CMAKE_SOURCE_DIR}/App/Resources/Translations/*.qm\")

# Sort the files for consistent output
list(SORT QM_FILES)

# Start QRC content
set(QRC_CONTENT \"<RCC>\\n    <qresource prefix=\\\"/translations\\\">\\n\")

# Add each .qm file
foreach(qm_file \${QM_FILES})
    get_filename_component(qm_name \${qm_file} NAME)
    string(APPEND QRC_CONTENT \"        <file>\${qm_name}</file>\\n\")
endforeach()

# End QRC content
string(APPEND QRC_CONTENT \"    </qresource>\\n</RCC>\\n\")

# Write the QRC file
file(WRITE \${QRC_FILE} \${QRC_CONTENT})

# Report generation
list(LENGTH QM_FILES QM_COUNT)
message(STATUS \"Generated translations.qrc with \${QM_COUNT} files\")
foreach(qm_file \${QM_FILES})
    get_filename_component(qm_name \${qm_file} NAME)
    message(STATUS \"  - \${qm_name}\")
endforeach()
")

# Custom target to regenerate translations.qrc using native CMake
add_custom_target(generate_translations_qrc
    COMMAND ${CMAKE_COMMAND} -P ${QRC_GENERATOR_SCRIPT}
    COMMENT "Generating translations.qrc with CMake"
    DEPENDS lrelease
)

# Print translation information
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

# Link a target against wiredpanda_lib with --whole-archive to prevent the
# linker from discarding element .o files whose only external symbols are
# static-initialization registrations (ElementInfo<T>::registered).
function(link_wiredpanda_whole_archive target)
    if(MSVC)
        target_link_libraries(${target} PRIVATE wiredpanda_lib)
        target_link_options(${target} PRIVATE /WHOLEARCHIVE:wiredpanda_lib)
    elseif(APPLE)
        target_link_libraries(${target} PRIVATE wiredpanda_lib)
        target_link_options(${target} PRIVATE "LINKER:-force_load,$<TARGET_FILE:wiredpanda_lib>")
    else()
        target_link_libraries(${target} PRIVATE
            -Wl,--whole-archive wiredpanda_lib -Wl,--no-whole-archive)
    endif()
endfunction()

# ========= LIB ===================================

add_library(wiredpanda_lib STATIC ${SOURCES} ${HEADERS})
target_link_libraries(wiredpanda_lib PUBLIC ${QT_LIBS} nlohmann_json::nlohmann_json nlohmann_json_schema_validator)

# Treat external dependencies as system headers to suppress clazy warnings
target_include_directories(wiredpanda_lib SYSTEM PUBLIC
    ${CMAKE_BINARY_DIR}/_deps/json-src/include
    ${CMAKE_BINARY_DIR}/_deps/json-schema-validator-src/src
)

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

# Copy JSON schema to executable directory after build
add_custom_command(TARGET wiredpanda POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/MCP/schema-mcp.json
        $<TARGET_FILE_DIR:wiredpanda>/schema-mcp.json
    COMMENT "Copying JSON schema to executable directory"
    VERBATIM
)

# Run windeployqt on Windows to copy Qt dependencies
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

# Ensure translations are compiled before building the main executable
if(Qt6LinguistTools_FOUND)
    add_dependencies(wiredpanda lrelease)
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
target_link_libraries(test_utils PUBLIC wiredpanda_lib)
target_compile_definitions(test_utils PRIVATE CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/Tests)

# Helper library for shared circuit builders (register file, memory tests)
add_library(memory_helpers STATIC ${MEMORY_HELPERS_SOURCES} ${MEMORY_HELPERS_HEADERS})
target_link_libraries(memory_helpers PUBLIC wiredpanda_lib test_utils)

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

add_test(NAME TestArduino COMMAND test_wiredpanda TestArduino WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestAudioBox COMMAND test_wiredpanda TestAudioBox WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
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
add_test(NAME TestFiles COMMAND test_wiredpanda TestFiles WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGeometry COMMAND test_wiredpanda TestGeometry WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestGraphicelementAdvanced COMMAND test_wiredpanda TestGraphicelementAdvanced WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestIC COMMAND test_wiredpanda TestIC WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestIcons COMMAND test_wiredpanda TestIcons WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestInputElements COMMAND test_wiredpanda TestInputElements WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestInputRotary COMMAND test_wiredpanda TestInputRotary WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestLED COMMAND test_wiredpanda TestLED WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
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
add_test(NAME TestWorkspace COMMAND test_wiredpanda TestWorkspace WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME TestWorkspaceFileops COMMAND test_wiredpanda TestWorkspaceFileops WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

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
        TestLevel9SingleCycleCPU8Bit)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# IC-specific deep tests - Integration category
foreach(TEST TestDecoder8To256 TestMemorySettlingTime TestRAMCell1Bit TestSequential
        TestMUXDEMUXComprehensive)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# File and Simulation Tests - Integration category
foreach(TEST TestArduino TestFeedback TestFiles TestIC TestSystemVerilogExport TestSimulation TestWorkspace
        TestWorkspaceFileops)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# Element and Component Unit Tests - Unit category
foreach(TEST TestIcons TestWaveform TestAudioBox TestBuzzer TestClock TestClocksAdvanced
        TestComponents TestDisplays TestElementLabel TestElementProperties TestFeatures
        TestGeometry TestGraphicelementAdvanced TestInputElements TestInputRotary TestLED
        TestLogicGates TestMultiplexing TestSequentialLogic TestStatusOps TestTruthTable)
    set_tests_properties(${TEST} PROPERTIES LABELS "unit")
endforeach()

# Factory and Core Element Tests - Unit category
foreach(TEST TestElementFactory TestElementLogic TestElementLogicErrors TestNodeLogic)
    set_tests_properties(${TEST} PROPERTIES LABELS "unit")
endforeach()

# Connection and Scene Tests - Integration category
foreach(TEST TestConnectionSerialization TestConnectionValidity TestConnections TestScene
        TestSceneConnections TestSceneState TestSceneUndoredo)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# Tests that touch the system clipboard must not run in parallel (Windows OLE contention).
set_tests_properties(TestScene TestSceneUndoredo PROPERTIES RESOURCE_LOCK clipboard)

# Serialization and Command Tests - Integration category
foreach(TEST TestSerialization TestSimulationBlocker TestDialogs)
    set_tests_properties(${TEST} PROPERTIES LABELS "integration")
endforeach()

# Core Functionality Tests - Unit category
foreach(TEST TestCommands TestCommon TestEnums TestPriorities TestRecentFiles TestSettings
        TestThemeManager)
    set_tests_properties(${TEST} PROPERTIES LABELS "unit")
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
