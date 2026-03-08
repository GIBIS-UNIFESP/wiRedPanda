cmake_minimum_required(VERSION 3.16)
project(wiredpanda VERSION 4.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt version selection: "auto" (default), "5", or "6"
set(QT_VERSION "auto" CACHE STRING "Qt version to use: auto (default), 5, or 6")
set_property(CACHE QT_VERSION PROPERTY STRINGS "auto" "5" "6")
message(STATUS "Qt version selection: ${QT_VERSION}")

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
    find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)
else()
    if(QT_VERSION STREQUAL "5")
        # Force Qt5
        find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
        set(QT_VERSION_MAJOR 5)
        set(QT_LIBS Qt5::Core Qt5::Gui Qt5::PrintSupport Qt5::Multimedia Qt5::Widgets Qt5::Svg Qt5::Test)
        message(STATUS "Using Qt5 (forced)")
    elseif(QT_VERSION STREQUAL "6")
        # Force Qt6
        find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
        set(QT_VERSION_MAJOR 6)
        set(QT_LIBS Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)
        message(STATUS "Using Qt6 (forced)")
    else()
        # Auto-detect: prefer Qt6, fallback to Qt5
        find_package(Qt6 6.2 QUIET COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
        if(Qt6_FOUND)
            set(QT_VERSION_MAJOR 6)
            set(QT_LIBS Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)
            message(STATUS "Auto-detected Qt6")
        else()
            find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
            set(QT_VERSION_MAJOR 5)
            set(QT_LIBS Qt5::Core Qt5::Gui Qt5::PrintSupport Qt5::Multimedia Qt5::Widgets Qt5::Svg Qt5::Test)
            message(STATUS "Auto-detected Qt5 (Qt6 not found)")
        endif()
    endif()
endif()

if((QT_VERSION_MAJOR EQUAL 5 AND Qt5_VERSION VERSION_LESS 5.15) OR
   (QT_VERSION_MAJOR EQUAL 6 AND Qt6_VERSION VERSION_LESS 6.2))
    message(FATAL_ERROR "Minimum required Qt version is 5.15 for Qt5 or 6.2 for Qt6.")
endif()

add_compile_definitions(
    APP_VERSION="${PROJECT_VERSION}"
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    QT_MESSAGELOGCONTEXT
)

if(MSVC)
    add_compile_options(/W4 /external:W0 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
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
option(ENABLE_UNITY_BUILD "Enable Unity builds for faster compilation" ON)
set(UNITY_BUILD_BATCH_SIZE 8 CACHE STRING "Number of source files to combine in each unity file")
if(ENABLE_UNITY_BUILD)
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

if(QT_VERSION_MAJOR EQUAL 5)
    set(CMAKE_OSX_ARCHITECTURES x86_64)
else()
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
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
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5LinguistTools QUIET)
    if(Qt5LinguistTools_FOUND)
        foreach(ts_file ${TS_FILES})
            get_filename_component(qm_file ${ts_file} NAME_WE)
            get_filename_component(ts_dir ${ts_file} DIRECTORY)
            set(qm_path ${ts_dir}/${qm_file}.qm)
            add_custom_command(
                OUTPUT ${qm_path}
                COMMAND ${Qt5_LRELEASE_EXECUTABLE} ${ts_file} -qm ${qm_path}
                DEPENDS ${ts_file}
                COMMENT "Compiling ${qm_file}.qm"
                VERBATIM
            )
            list(APPEND QM_FILES ${qm_path})
        endforeach()
    endif()
    qt5_add_resources(RESOURCES_RCC ${RESOURCES})
else()
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
endif()

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
if(Qt5LinguistTools_FOUND OR Qt6LinguistTools_FOUND)
    # Custom target to update translation source files
    if(QT_VERSION_MAJOR EQUAL 5)
        add_custom_target(lupdate
            COMMAND Qt5::lupdate -tr-function-alias tr+=PANDACEPTION -extensions cpp,h,ui -I ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/App -ts ${TS_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Updating translation source files"
        )
    else()
        add_custom_target(lupdate
            COMMAND Qt6::lupdate -tr-function-alias tr+=PANDACEPTION -extensions cpp,h,ui -I ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/App -ts ${TS_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Updating translation source files"
        )
    endif()

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

# ========= LIB ===================================

add_library(wiredpanda_lib STATIC ${SOURCES} ${HEADERS})
target_link_libraries(wiredpanda_lib PUBLIC ${QT_LIBS})

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
target_link_libraries(wiredpanda PRIVATE wiredpanda_lib wiredpanda_resources)

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
if(Qt5LinguistTools_FOUND OR Qt6LinguistTools_FOUND)
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

# ========= TESTS ===================================

# Define the test executables
set(TEST_EXECUTABLES
    TestCommands
    TestElements
    TestFiles
    TestIcons
    TestLogicElements
    TestSimulation
    TestWaveform
)

# Create individual test executables
foreach(test_name ${TEST_EXECUTABLES})
    add_executable(${test_name} Tests/${test_name}.cpp Tests/${test_name}.h)
    target_link_libraries(${test_name} PRIVATE wiredpanda_lib wiredpanda_resources)

    target_compile_definitions(${test_name} PRIVATE CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/Tests)

    if(ENABLE_PCH)
        target_precompile_headers(${test_name} REUSE_FROM wiredpanda_lib)
    endif()
endforeach()

# Add CTest support
enable_testing()

# Register each test executable with CTest
foreach(test_name ${TEST_EXECUTABLES})
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# ========= DOCUMENTATION ===================================

find_package(Doxygen OPTIONAL_COMPONENTS dot)
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
