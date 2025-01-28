cmake_minimum_required(VERSION 3.16)
project(wiredpanda VERSION 4.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 QUIET COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_DIR ${Qt6_DIR})
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)
else()
    find_package(Qt5 5.12 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
    set(QT_VERSION_MAJOR 5)
    set(QT_DIR ${Qt5_DIR})
    set(QT_LIBS Qt5::Core Qt5::Gui Qt5::PrintSupport Qt5::Multimedia Qt5::Widgets Qt5::Svg Qt5::Test)
endif()

if(QT_VERSION_MAJOR EQUAL 5 AND Qt5_VERSION VERSION_LESS 5.12)
    message(FATAL_ERROR "Qt 5.12 or newer is required.")
elseif(QT_VERSION_MAJOR EQUAL 6 AND Qt6_VERSION VERSION_LESS 6.2)
    message(FATAL_ERROR "For Qt6, the minimum version is 6.2.0.")
endif()

add_compile_definitions(
    APP_VERSION="${PROJECT_VERSION}"
    QT_DEPRECATED_WARNINGS
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
    QT_MESSAGELOGCONTEXT
)

if (MSVC)
    add_compile_options(/W4 /external:I ${QT_DIR} /external:W0 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_program(CCACHE_BIN ccache)

if(CCACHE_BIN)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_BIN})
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_BIN})
endif()

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

include(CMakeSources.cmake)

if(QT_VERSION_MAJOR EQUAL 5)
    qt5_add_resources(RESOURCES_RCC ${RESOURCES})
else()
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

# ========= APP ===================================

add_executable(wiredpanda
    app/main.cpp
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES_RCC}
)

target_link_libraries(wiredpanda PRIVATE ${QT_LIBS})

if(WIN32)
    set(WINDOWS_APP_NAME "wiRedPanda - Logic Circuit Simulator")
    set(RC_FILE "resources/wpanda.ico")

    set_target_properties(wiredpanda PROPERTIES
        WIN32_EXECUTABLE ON
        RUNTIME_OUTPUT_NAME "${WINDOWS_APP_NAME}"
    )

    if(EXISTS ${RC_FILE})
        target_sources(wiredpanda PRIVATE ${RC_FILE})
    endif()
endif()

if(MSVC)
    target_precompile_headers(wiredpanda PRIVATE pch.h)
endif()

# ========= TESTS ===================================

file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
file(GLOB_RECURSE TEST_HEADERS "test/*.h")

add_executable(test
    ${TEST_SOURCES}
    ${TEST_HEADERS}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${RESOURCES_RCC}
)

target_link_libraries(test PRIVATE ${QT_LIBS})
target_precompile_headers(test PRIVATE pch.h)
target_compile_definitions(test PRIVATE CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/test)

if(MSVC)
    target_precompile_headers(test PRIVATE pch.h)
endif()
