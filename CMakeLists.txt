cmake_minimum_required(VERSION 3.16)
project(wiredpanda VERSION 4.2.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable conforming preprocessor for MSVC to support __VA_OPT__
if(MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(EMSCRIPTEN)
    find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)
else()
    find_package(Qt6 6.2 QUIET COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
        set(QT_LIBS Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)
    else()
        find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
        set(QT_VERSION_MAJOR 5)
        set(QT_LIBS Qt5::Core Qt5::Gui Qt5::PrintSupport Qt5::Multimedia Qt5::Widgets Qt5::Svg Qt5::Test)
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
    add_link_options("-fuse-ld=mold")
endif()

# Coverage support
if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
    message(STATUS "Coverage enabled")
endif()

# Sanitizer support
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(ENABLE_MEMORY_SANITIZER)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
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
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
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
    endif()
endif()

if(QT_VERSION_MAJOR EQUAL 5)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
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
file(GLOB_RECURSE TS_FILES "app/resources/translations/wpanda_*.ts")

# Initialize QM_FILES list
set(QM_FILES "")

# Create QM files in source directory so QRC can find them
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5LinguistTools QUIET)
    if(Qt5LinguistTools_FOUND)
        foreach(ts_file ${TS_FILES})
            get_filename_component(qm_file ${ts_file} NAME_WE)
            get_filename_component(ts_dir ${ts_file} DIRECTORY)
            set(qm_path "${ts_dir}/${qm_file}.qm")
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
            set(qm_path "${ts_dir}/${qm_file}.qm")
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

include_directories(
    ${CMAKE_BINARY_DIR}
    app
    app/arduino
    app/element
    app/logicelement
    app/nodes
)

# ========= TRANSLATIONS ===========================

# Find available translation files
file(GLOB_RECURSE TS_FILES "app/resources/translations/wpanda_*.ts")

# Custom targets for translation management
if(Qt5LinguistTools_FOUND OR Qt6LinguistTools_FOUND)
    # Custom target to update translation source files
    if(QT_VERSION_MAJOR EQUAL 5)
        add_custom_target(lupdate
            COMMAND Qt5::lupdate -tr-function-alias tr+=PANDACEPTION ${CMAKE_SOURCE_DIR}/app -ts ${TS_FILES}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Updating translation source files"
        )
    else()
        add_custom_target(lupdate
            COMMAND Qt6::lupdate -tr-function-alias tr+=PANDACEPTION ${CMAKE_SOURCE_DIR}/app -ts ${TS_FILES}
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
set(QRC_FILE \"${CMAKE_SOURCE_DIR}/app/resources/translations/translations.qrc\")

# Get list of .qm files in translations directory
file(GLOB QM_FILES \"${CMAKE_SOURCE_DIR}/app/resources/translations/*.qm\")

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

# ========= LIB ===================================

add_library(wiredpanda_lib STATIC ${SOURCES} ${HEADERS} ${FORMS})
target_link_libraries(wiredpanda_lib PUBLIC ${QT_LIBS})

if(NOT CCACHE_BIN)
    target_precompile_headers(wiredpanda_lib PRIVATE pch.h)
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
    qt_add_executable(wiredpanda app/main.cpp ${RESOURCES_RCC})
else()
    add_executable(wiredpanda app/main.cpp ${RESOURCES_RCC})
endif()
target_link_libraries(wiredpanda PRIVATE wiredpanda_lib)

# Ensure translations are compiled before building the main executable
if(Qt5LinguistTools_FOUND OR Qt6LinguistTools_FOUND)
    add_dependencies(wiredpanda lrelease)
endif()

if(NOT CCACHE_BIN)
    target_precompile_headers(wiredpanda PRIVATE pch.h)
endif()

if(WIN32)
    set(WINDOWS_APP_NAME "wiRedPanda - Logic Circuit Simulator")
    set(RC_FILE "resources/wpanda.ico")

    set_target_properties(wiredpanda PROPERTIES
        WIN32_EXECUTABLE ON
        RUNTIME_OUTPUT_NAME "wiredpanda"
        # Enhanced Windows metadata
        VERSION "${PROJECT_VERSION}"
        DESCRIPTION "wiRedPanda - Logic Circuit Simulator"
        COMPANY_NAME "GIBIS-UNIFESP"
        FILE_DESCRIPTION "wiRedPanda - Logic Circuit Simulator"
        LEGAL_COPYRIGHT "GIBIS-UNIFESP and the wiRedPanda contributors"
        ORIGINAL_FILENAME "wiredpanda.exe"
        PRODUCT_NAME "wiRedPanda - Logic Circuit Simulator"
    )

    if(EXISTS ${RC_FILE})
        target_sources(wiredpanda PRIVATE ${RC_FILE})
    endif()
endif()

# ========= TESTS ===================================

file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
file(GLOB_RECURSE TEST_HEADERS "test/*.h")

add_executable(wiredpanda-test ${TEST_SOURCES} ${TEST_HEADERS} ${RESOURCES_RCC})
target_link_libraries(wiredpanda-test PRIVATE wiredpanda_lib)

# Ensure translations are compiled before building the test executable
if(Qt5LinguistTools_FOUND OR Qt6LinguistTools_FOUND)
    add_dependencies(wiredpanda-test lrelease)
endif()
target_compile_definitions(wiredpanda-test PRIVATE CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/test)

if(NOT CCACHE_BIN)
    target_precompile_headers(wiredpanda-test PRIVATE pch.h)
endif()
