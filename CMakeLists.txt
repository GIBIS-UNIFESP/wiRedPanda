cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

project(WiredPanda
    VERSION 1.0
    DESCRIPTION "Logic Port teaching tool"
    LANGUAGES CXX
)

# wiRedPanda version
set(APP_VERSION "3.0")
option(BUILD_TEST "BUILD_TEST" 0)

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.14.0")

set(CMAKE_AUTOMOC ON)
set(AUTOMOC_MOC_OPTIONS -Muri=org.GIBIS-UNIFESP.wiRedPanda)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_compile_definitions(
    QT_USE_QSTRINGBUILDER
    QT_NO_CAST_TO_ASCII
    QT_STRICT_ITERATORS
    QT_NO_CAST_FROM_BYTEARRAY
    QT_USE_FAST_OPERATOR_PLUS
)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED
    Charts
    Core
    Gui
    Multimedia
    PrintSupport
    Widgets
)

set( 
    WPANDA_LIBS
    Qt5::Charts
    Qt5::Core
    Qt5::Gui
    Qt5::Multimedia
    Qt5::PrintSupport
    Qt5::Widgets
)

find_package (ECM ${KF5_MIN_VERSION} NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(cmake/Git.cmake)
include(cmake/Format.cmake)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
include_directories(./app/element)
include_directories(./app/nodes)
include_directories(./app)

add_subdirectory(app)
if(BUILD_TEST)
    find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Test )
    add_subdirectory(test)
endif()
