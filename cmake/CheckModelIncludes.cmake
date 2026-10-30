# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Checks that no file of the model includes a presentation header (CMakeSources.cmake's MODEL_* and PRESENTATION_*
# lists): presentation may include the model, never the reverse.
#
#   cmake -DCONFIG=<file> -P CheckModelIncludes.cmake
#
# The configuration file sets:
#   CHECK_SOURCE_DIR          the source root, against which "App/..." includes resolve
#   CHECK_MODEL_FILES         the model's sources and headers (absolute paths)
#   CHECK_PRESENTATION_FILES  presentation's sources and headers (absolute paths)
#
# An include in quotes names a file beside the including one or under the source root, the first that exists, as the
# compiler finds it; one in angle brackets names a file under the source root. Every failure is printed, then a fatal
# error fails the build.

cmake_minimum_required(VERSION 3.27)

if(NOT DEFINED CONFIG)
    message(FATAL_ERROR "CheckModelIncludes.cmake: pass -DCONFIG=<file>")
endif()
include("${CONFIG}")
# An empty list joined to another leaves an empty item.
list(REMOVE_ITEM CHECK_MODEL_FILES "")
list(REMOVE_ITEM CHECK_PRESENTATION_FILES "")

foreach(file IN LISTS CHECK_PRESENTATION_FILES)
    cmake_path(NORMAL_PATH file)
    set("presentation_${file}" TRUE)
endforeach()

# Whether the file an include names is a presentation file.
function(names_presentation out file delimiter name)
    set(candidates "${CHECK_SOURCE_DIR}/${name}")
    if(delimiter STREQUAL "\"")
        cmake_path(GET file PARENT_PATH dir)
        list(PREPEND candidates "${dir}/${name}")
    endif()
    set(result FALSE)
    foreach(candidate IN LISTS candidates)
        if(EXISTS "${candidate}")
            cmake_path(NORMAL_PATH candidate)
            if(DEFINED "presentation_${candidate}")
                set(result TRUE)
            endif()
            break()
        endif()
    endforeach()
    set(${out} ${result} PARENT_SCOPE)
endfunction()

set(failures "")

foreach(file IN LISTS CHECK_MODEL_FILES)
    cmake_path(RELATIVE_PATH file BASE_DIRECTORY "${CHECK_SOURCE_DIR}" OUTPUT_VARIABLE relative)
    file(STRINGS "${file}" lines REGEX "^[ \t]*#[ \t]*include[ \t]*[<\"]")
    foreach(line IN LISTS lines)
        if(line MATCHES "^[ \t]*#[ \t]*include[ \t]*([<\"])([^>\"]+)([>\"])")
            set(delimiter "${CMAKE_MATCH_1}")
            set(name "${CMAKE_MATCH_2}")
            set(text "${CMAKE_MATCH_1}${CMAKE_MATCH_2}${CMAKE_MATCH_3}")
            names_presentation(presentation "${file}" "${delimiter}" "${name}")
            if(presentation)
                list(APPEND failures
                    "${relative}: #include ${text} is a presentation header, and the model includes none")
            endif()
        endif()
    endforeach()
endforeach()

# Each failure on a line of its own, as printed: an error's message would be wrapped.
if(failures)
    foreach(failure IN LISTS failures)
        message(NOTICE "${failure}")
    endforeach()
    list(LENGTH failures count)
    message(FATAL_ERROR "The model includes presentation (${count})")
endif()
