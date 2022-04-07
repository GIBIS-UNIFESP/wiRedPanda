# Copyright 2022, Patrick José Pereira <patrickelectric@gmail.com>
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Git)

if(NOT GIT_FOUND OR NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    return()
endif()

# Fetch the necessary git variables
execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%aI
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_VERSION_DATE
                OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${GIT_EXECUTABLE} describe --match "[v,t]*" --tags --abbrev=0
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_TAG
                OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND ${GIT_EXECUTABLE} remote get-url origin
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_URL
                OUTPUT_STRIP_TRAILING_WHITESPACE)

add_compile_definitions(
    GIT_VERSION="${GIT_VERSION}"
    GIT_VERSION_DATE="${GIT_VERSION_DATE}"
    GIT_TAG="${GIT_TAG}"
    GIT_URL="${GIT_URL}"
)