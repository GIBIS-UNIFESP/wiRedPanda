# cmake/AutoDetectQt.cmake
#
# Auto-detects a Qt 6 installation and sets CMAKE_PREFIX_PATH when none of the
# standard discovery mechanisms have been provided by the caller.
#
# Resolution order (first match wins):
#   1. CMAKE_PREFIX_PATH already set (command-line, preset, or cached from a
#      previous configure run)
#   2. Qt6_DIR / QTDIR environment variables
#   3. Scan well-known installation roots and pick the newest Qt 6.x found,
#      matching the active compiler on Windows (MSVC vs MinGW)
#
# The detected path is written to the CMake cache so that re-runs of cmake skip
# the scan and remain stable.  Developers with non-standard paths can override
# at any time with -DCMAKE_PREFIX_PATH=<path>.

# 1. Already configured — nothing to do.
if(CMAKE_PREFIX_PATH OR Qt6_DIR)
    return()
endif()

# 2. Environment variables.
foreach(_envvar Qt6_DIR QTDIR Qt6_ROOT)
    if(DEFINED ENV{${_envvar}})
        list(APPEND CMAKE_PREFIX_PATH "$ENV{${_envvar}}")
        message(STATUS "Qt6: using path from ${_envvar} env: $ENV{${_envvar}}")
        return()
    endif()
endforeach()

# 3. Scan common installation roots.

set(_search_roots)

if(WIN32)
    # Qt Online Installer default: C:\Qt, D:\Qt, etc.
    foreach(_drive "C" "D" "E")
        if(IS_DIRECTORY "${_drive}:/Qt")
            list(APPEND _search_roots "${_drive}:/Qt")
        endif()
    endforeach()
    # Per-user installs
    if(DEFINED ENV{USERPROFILE} AND IS_DIRECTORY "$ENV{USERPROFILE}/Qt")
        list(APPEND _search_roots "$ENV{USERPROFILE}/Qt")
    endif()
elseif(APPLE)
    if(IS_DIRECTORY "$ENV{HOME}/Qt")
        list(APPEND _search_roots "$ENV{HOME}/Qt")
    endif()
else()
    if(IS_DIRECTORY "$ENV{HOME}/Qt")
        list(APPEND _search_roots "$ENV{HOME}/Qt")
    endif()
    foreach(_path "/opt/Qt" "/opt/qt6")
        if(IS_DIRECTORY "${_path}")
            list(APPEND _search_roots "${_path}")
        endif()
    endforeach()
endif()

if(NOT _search_roots)
    return()
endif()

# On Windows, narrow the variant glob to match the active compiler so we don't
# accidentally pick a MinGW Qt when building with MSVC or vice versa.
if(WIN32)
    if(MSVC)
        set(_variant_globs "msvc*_64" "msvc*")
    else()
        set(_variant_globs "mingw*_64" "mingw*")
    endif()
elseif(APPLE)
    set(_variant_globs "macos")
else()
    set(_variant_globs "gcc_64" "linux*" "*")
endif()

set(_candidates)
foreach(_root ${_search_roots})
    foreach(_variant ${_variant_globs})
        file(GLOB _found "${_root}/6.*/${_variant}")
        list(APPEND _candidates ${_found})
    endforeach()
endforeach()

if(NOT _candidates)
    return()
endif()

# Pick newest version (natural sort handles 6.9 vs 6.10 correctly).
list(SORT _candidates COMPARE NATURAL ORDER DESCENDING)
list(GET _candidates 0 _qt_prefix)

set(CMAKE_PREFIX_PATH "${_qt_prefix}" CACHE PATH
    "Qt installation prefix (auto-detected; set explicitly to override)")
message(STATUS "Qt6 auto-detected: ${_qt_prefix}")
message(STATUS "  Override: cmake -DCMAKE_PREFIX_PATH=<path/to/Qt/6.x/platform> ...")
