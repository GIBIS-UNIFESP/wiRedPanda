# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Checks that no file of the model includes a presentation header (CMakeSources.cmake's MODEL_* and PRESENTATION_*
# lists), directly or through the rest of the application's headers: presentation may include the model, never the
# reverse.
#
#   cmake -DCONFIG=<file> -P CheckModelIncludes.cmake
#
# The configuration file sets:
#   CHECK_SOURCE_DIR          the source root, against which "App/..." includes resolve
#   CHECK_MODEL_FILES         the model's sources and headers (absolute paths)
#   CHECK_PRESENTATION_FILES  presentation's sources and headers (absolute paths)
#
# An include in quotes names a file beside the including one or under the source root, the first that exists, as the
# compiler finds it; one in angle brackets names a file under the source root. An include of a file that is neither the
# model's nor presentation's is followed through what that file includes, to any depth, and fails when it reaches
# presentation, naming the files between; a model file it reaches is checked on its own. Every failure is printed, then
# a fatal error fails the build.

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
foreach(file IN LISTS CHECK_MODEL_FILES)
    cmake_path(NORMAL_PATH file)
    set("model_${file}" TRUE)
endforeach()

# The file under the source root that an include of \a name in \a delimiter from \a file names; empty when none does.
function(resolve out file delimiter name)
    set(candidates "${CHECK_SOURCE_DIR}/${name}")
    if(delimiter STREQUAL "\"")
        cmake_path(GET file PARENT_PATH dir)
        list(PREPEND candidates "${dir}/${name}")
    endif()
    foreach(candidate IN LISTS candidates)
        if(EXISTS "${candidate}" AND NOT IS_DIRECTORY "${candidate}")
            cmake_path(NORMAL_PATH candidate)
            set(${out} "${candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${out} "" PARENT_SCOPE)
endfunction()

# The project files \a file includes, each as "<the include as written>|<the file>", read once per file.
function(includes_of out file)
    get_property(known GLOBAL PROPERTY "includes_${file}" SET)
    if(NOT known)
        set(entries "")
        file(STRINGS "${file}" lines REGEX "^[ \t]*#[ \t]*include[ \t]*[<\"]")
        foreach(line IN LISTS lines)
            if(line MATCHES "^[ \t]*#[ \t]*include[ \t]*([<\"])([^>\"]+)([>\"])")
                set(text "${CMAKE_MATCH_1}${CMAKE_MATCH_2}${CMAKE_MATCH_3}")
                resolve(path "${file}" "${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")
                if(path)
                    list(APPEND entries "${text}|${path}")
                endif()
            endif()
        endforeach()
        set_property(GLOBAL PROPERTY "includes_${file}" "${entries}")
    endif()
    get_property(entries GLOBAL PROPERTY "includes_${file}")
    set(${out} "${entries}" PARENT_SCOPE)
endfunction()

# Splits an includes_of() entry into the include as written and the file.
macro(split_entry entry text path)
    string(FIND "${entry}" "|" bar REVERSE)
    string(SUBSTRING "${entry}" 0 ${bar} ${text})
    math(EXPR after "${bar} + 1")
    string(SUBSTRING "${entry}" ${after} -1 ${path})
endmacro()

# Follows what \a start, a file of neither the model nor presentation, includes, through other such files. Returns in
# \a out_chain the files from \a start to the first presentation header reached, relative to the source root and joined
# by " -> "; empty when it reaches none.
function(chain_to_presentation out_chain start)
    set(queue "${start}")
    set("seen_${start}" TRUE)
    while(queue)
        list(POP_FRONT queue current)
        includes_of(entries "${current}")
        foreach(entry IN LISTS entries)
            split_entry("${entry}" text path)
            if(DEFINED "seen_${path}")
                continue()
            endif()
            set("seen_${path}" TRUE)
            set("parent_${path}" "${current}")
            if(DEFINED "presentation_${path}")
                set(chain "")
                set(node "${path}")
                while(TRUE)
                    cmake_path(RELATIVE_PATH node BASE_DIRECTORY "${CHECK_SOURCE_DIR}" OUTPUT_VARIABLE shown)
                    list(PREPEND chain "${shown}")
                    if(node STREQUAL start)
                        break()
                    endif()
                    set(node "${parent_${node}}")
                endwhile()
                list(JOIN chain " -> " joined)
                set(${out_chain} "${joined}" PARENT_SCOPE)
                return()
            endif()
            if(NOT DEFINED "model_${path}")
                list(APPEND queue "${path}")
            endif()
        endforeach()
    endwhile()
    set(${out_chain} "" PARENT_SCOPE)
endfunction()

set(failures "")

foreach(file IN LISTS CHECK_MODEL_FILES)
    cmake_path(NORMAL_PATH file)
    cmake_path(RELATIVE_PATH file BASE_DIRECTORY "${CHECK_SOURCE_DIR}" OUTPUT_VARIABLE relative)
    includes_of(entries "${file}")
    foreach(entry IN LISTS entries)
        split_entry("${entry}" text path)
        if(DEFINED "presentation_${path}")
            list(APPEND failures
                "${relative}: #include ${text} is a presentation header, and the model includes none")
        elseif(NOT DEFINED "model_${path}")
            chain_to_presentation(chain "${path}")
            if(chain)
                list(APPEND failures
                    "${relative}: #include ${text} reaches a presentation header, and the model includes none: ${chain}")
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
