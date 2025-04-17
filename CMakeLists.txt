cmake_minimum_required(VERSION 3.16)
project(wiredpanda VERSION 4.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 QUIET COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Multimedia Qt6::Widgets Qt6::Svg Qt6::Test)
else()
    find_package(Qt5 5.12 REQUIRED COMPONENTS Core Gui PrintSupport Multimedia Widgets Svg Test)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBS Qt5::Core Qt5::Gui Qt5::PrintSupport Qt5::Multimedia Qt5::Widgets Qt5::Svg Qt5::Test)
endif()

if((QT_VERSION_MAJOR EQUAL 5 AND Qt5_VERSION VERSION_LESS 5.12) OR
   (QT_VERSION_MAJOR EQUAL 6 AND Qt6_VERSION VERSION_LESS 6.2))
    message(FATAL_ERROR "Minimum required Qt version is 5.12 for Qt5 or 6.2 for Qt6.")
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

add_executable(wiredpanda app/main.cpp ${RESOURCES_RCC})
target_link_libraries(wiredpanda PRIVATE wiredpanda_lib)

if(NOT CCACHE_BIN)
    target_precompile_headers(wiredpanda PRIVATE pch.h)
endif()

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

# ========= TESTS ===================================

file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")
file(GLOB_RECURSE TEST_HEADERS "test/*.h")

add_executable(test ${TEST_SOURCES} ${TEST_HEADERS} ${RESOURCES_RCC})
target_link_libraries(test PRIVATE wiredpanda_lib)
target_compile_definitions(test PRIVATE CURRENTDIR=${CMAKE_CURRENT_SOURCE_DIR}/test)

if(NOT CCACHE_BIN)
    target_precompile_headers(test PRIVATE pch.h)
endif()
