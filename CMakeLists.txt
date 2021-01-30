cmake_minimum_required (VERSION 3.12 FATAL_ERROR)

project(WiredPanda
    VERSION 1.0
    DESCRIPTION "Logic Port teaching tool"
    LANGUAGES CXX
)

find_package (ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(cmake/Format.cmake)
