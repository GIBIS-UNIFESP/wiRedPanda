# cmake/AutoDetectQt.cmake
#
# Auto-detects a Qt 6 installation and appends its prefix to CMAKE_PREFIX_PATH
# when none of the standard discovery mechanisms have been provided.
#
# Resolution order (first match wins):
#   1. _QT_AUTO_PREFIX already cached from a prior configure run
#   2. CMAKE_PREFIX_PATH / Qt6_DIR already set (command-line, preset, kit)
#   3. Qt6_DIR / QTDIR / Qt6_ROOT environment variables
#   4. Scan well-known roots for Qt6Config.cmake, prefer the platform variant
#      that matches the active compiler, fall back to any available platform
#
# The detected path is stored in the INTERNAL cache variable _QT_AUTO_PREFIX
# so re-runs skip the scan entirely.  Override at any time with:
#   cmake -DCMAKE_PREFIX_PATH=<path/to/Qt/6.x/platform>

# 1. Re-use result from a prior configure run.
if(DEFINED _QT_AUTO_PREFIX)
    list(APPEND CMAKE_PREFIX_PATH "${_QT_AUTO_PREFIX}")
    return()
endif()

# 2. Already configured externally — nothing to do.
if(CMAKE_PREFIX_PATH OR Qt6_DIR)
    return()
endif()

# 3. Environment variables.
foreach(_envvar Qt6_DIR QTDIR Qt6_ROOT)
    if(DEFINED ENV{${_envvar}})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{${_envvar}}")
        message(STATUS "Qt6: path from ${_envvar}: $ENV{${_envvar}}")
        return()
    endif()
endforeach()

# 4. Scan common installation roots for Qt6Config.cmake.

set(_search_roots)
if(WIN32)
    foreach(_drive "C" "D" "E")
        if(IS_DIRECTORY "${_drive}:/Qt")
            list(APPEND _search_roots "${_drive}:/Qt")
        endif()
    endforeach()
    if(DEFINED ENV{USERPROFILE} AND IS_DIRECTORY "$ENV{USERPROFILE}/Qt")
        list(APPEND _search_roots "$ENV{USERPROFILE}/Qt")
    endif()
elseif(APPLE)
    if(IS_DIRECTORY "$ENV{HOME}/Qt")
        list(APPEND _search_roots "$ENV{HOME}/Qt")
    endif()
else()
    foreach(_path "$ENV{HOME}/Qt" "/opt/Qt" "/opt/qt6")
        if(IS_DIRECTORY "${_path}")
            list(APPEND _search_roots "${_path}")
        endif()
    endforeach()
endif()

if(NOT _search_roots)
    return()
endif()

# Find all platform prefixes that contain a valid Qt6Config.cmake.
# Qt installer layout: <root>/<version>/<platform>/lib/cmake/Qt6/Qt6Config.cmake
set(_all_prefixes)
foreach(_root ${_search_roots})
    file(GLOB _configs "${_root}/6.*/*/lib/cmake/Qt6/Qt6Config.cmake")
    foreach(_config ${_configs})
        # Walk 4 levels up to reach the platform prefix directory.
        cmake_path(GET _config PARENT_PATH _p)  # .../lib/cmake/Qt6
        cmake_path(GET _p     PARENT_PATH _p)   # .../lib/cmake
        cmake_path(GET _p     PARENT_PATH _p)   # .../lib
        cmake_path(GET _p     PARENT_PATH _p)   # <platform_prefix>
        list(APPEND _all_prefixes "${_p}")
    endforeach()
endforeach()

if(NOT _all_prefixes)
    return()
endif()

# Separate into compiler-matched (preferred) and everything else (fallback).
set(_preferred)
set(_fallback)
foreach(_prefix ${_all_prefixes})
    cmake_path(GET _prefix FILENAME _platform)
    if(WIN32)
        if(MSVC AND _platform MATCHES "msvc")
            list(APPEND _preferred "${_prefix}")
        elseif(NOT MSVC AND _platform MATCHES "mingw")
            list(APPEND _preferred "${_prefix}")
        else()
            list(APPEND _fallback "${_prefix}")
        endif()
    elseif(APPLE AND _platform STREQUAL "macos")
        list(APPEND _preferred "${_prefix}")
    elseif(UNIX AND NOT APPLE AND _platform MATCHES "gcc_64|linux")
        list(APPEND _preferred "${_prefix}")
    else()
        list(APPEND _fallback "${_prefix}")
    endif()
endforeach()

set(_candidates ${_preferred})
if(NOT _candidates)
    set(_candidates ${_fallback})
endif()

# Pick newest version (natural sort handles 6.9 vs 6.10 correctly).
list(SORT _candidates COMPARE NATURAL ORDER DESCENDING)
list(GET _candidates 0 _qt_prefix)

# Append to CMAKE_PREFIX_PATH as a normal variable so find_package sees it
# in this configure run, and cache the result separately for re-run stability.
list(APPEND CMAKE_PREFIX_PATH "${_qt_prefix}")
set(_QT_AUTO_PREFIX "${_qt_prefix}" CACHE INTERNAL "Auto-detected Qt prefix")

message(STATUS "Qt6 auto-detected: ${_qt_prefix}")
message(STATUS "  Override: cmake -DCMAKE_PREFIX_PATH=<path/to/Qt/6.x/platform> ...")
