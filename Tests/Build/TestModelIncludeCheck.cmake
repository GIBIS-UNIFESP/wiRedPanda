# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Tests the check that the model includes no presentation header (cmake/CheckModelIncludes.cmake) on a small source
# tree it writes.
#
#   cmake -DCHECK_SCRIPT=<CheckModelIncludes.cmake> -DWORK_DIR=<dir> -P TestModelIncludeCheck.cmake

cmake_minimum_required(VERSION 3.27)

foreach(input CHECK_SCRIPT WORK_DIR)
    if(NOT DEFINED ${input})
        message(FATAL_ERROR "TestModelIncludeCheck.cmake: pass -D${input}=...")
    endif()
endforeach()

set(root "${WORK_DIR}/Source")
file(REMOVE_RECURSE "${WORK_DIR}")

set(model_files "")
set(presentation_files "")

function(source owner path content)
    file(WRITE "${root}/${path}" "${content}")
    list(APPEND ${owner}_files "${root}/${path}")
    set(${owner}_files "${${owner}_files}" PARENT_SCOPE)
endfunction()

source(presentation "App/Presentation/Body.h" "#pragma once\n")
source(presentation "App/Model/Painter.h" "#pragma once\n")
source(presentation "Shared.h" "#pragma once\n")
source(model "App/Model/Model.h" "#pragma once\n")
source(model "App/Model/Shared.h" "#pragma once\n")
source(model "App/Model/Clean.cpp"
    "#include \"App/Model/Model.h\"\n#include \"Model.h\"\n#include <QObject>\n#include <memory>\n"
    "#include \"Missing.h\"\n")
source(model "App/Model/Commented.cpp"
    "// #include \"App/Presentation/Body.h\"\n/* #include \"App/Presentation/Body.h\" */\n")
source(model "App/Model/SameName.cpp" "#include \"Shared.h\"\n")
source(model "App/Model/FromRoot.cpp" "#include \"App/Presentation/Body.h\"\n")
source(model "App/Model/Relative.cpp" "#include \"../Presentation/Body.h\"\n")
source(model "App/Model/FromHere.cpp" "#include \"Painter.h\"\n")
source(model "App/Model/Angled.cpp" "#include <App/Presentation/Body.h>\n")
source(model "App/Model/Spaced.cpp" "  #  include   \"App/Presentation/Body.h\"\n")
source(model "App/Model/Both.cpp"
    "#include \"App/Presentation/Body.h\"\n#include \"Model.h\"\n#include \"Painter.h\"\n")

set(failures "")
set(case_number 0)

# Runs the check over the given model files and returns its result and what it printed.
function(run_check out_result out_output files)
    math(EXPR case_number "${case_number} + 1")
    set(case_number ${case_number} PARENT_SCOPE)
    list(TRANSFORM files PREPEND "${root}/")
    set(config "${WORK_DIR}/case${case_number}.cmake")
    file(WRITE "${config}"
"set(CHECK_SOURCE_DIR \"${root}\")
set(CHECK_MODEL_FILES \"${files}\")
set(CHECK_PRESENTATION_FILES \"${presentation_files}\")
")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -DCONFIG=${config} -P "${CHECK_SCRIPT}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE output
    )
    set(${out_result} "${result}" PARENT_SCOPE)
    set(${out_output} "${output}" PARENT_SCOPE)
endfunction()

function(expect_pass name files)
    run_check(result output "${files}")
    set(case_number ${case_number} PARENT_SCOPE)
    if(NOT result EQUAL 0)
        set(failures "${failures}\n${name}: expected the check to pass; it printed:\n${output}" PARENT_SCOPE)
    endif()
endfunction()

# Expects the check to fail and to print each regular expression after the second argument.
function(expect_fail name files)
    run_check(result output "${files}")
    set(case_number ${case_number} PARENT_SCOPE)
    if(result EQUAL 0)
        set(failures "${failures}\n${name}: expected the check to fail; it passed" PARENT_SCOPE)
        return()
    endif()
    foreach(expected IN LISTS ARGN)
        if(NOT output MATCHES "${expected}")
            set(failures "${failures}\n${name}: expected \"${expected}\" in what the check printed:\n${output}"
                PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

expect_pass("Model, Qt, standard and missing headers pass" "App/Model/Model.h;App/Model/Clean.cpp")
expect_pass("A commented include passes" "App/Model/Commented.cpp")
expect_pass("A model header beside the file passes though the source root holds a presentation one of its name"
    "App/Model/SameName.cpp")
expect_fail("A presentation header from the source root fails" "App/Model/FromRoot.cpp"
    "App/Model/FromRoot.cpp: #include \"App/Presentation/Body.h\" is a presentation header")
expect_fail("A presentation header by a relative path fails" "App/Model/Relative.cpp"
    "App/Model/Relative.cpp: #include \"\\.\\./Presentation/Body.h\" is a presentation header")
expect_fail("A presentation header beside the file fails" "App/Model/FromHere.cpp"
    "App/Model/FromHere.cpp: #include \"Painter.h\" is a presentation header")
expect_fail("A presentation header in angle brackets fails" "App/Model/Angled.cpp"
    "App/Model/Angled.cpp: #include <App/Presentation/Body.h> is a presentation header")
expect_fail("An include spaced out fails the same" "App/Model/Spaced.cpp"
    "App/Model/Spaced.cpp: #include \"App/Presentation/Body.h\" is a presentation header")
expect_fail("Every failure of a file is reported" "App/Model/Both.cpp"
    "App/Model/Both.cpp: #include \"App/Presentation/Body.h\"" "App/Model/Both.cpp: #include \"Painter.h\"")
expect_fail("Every failing file is reported" "App/Model/Clean.cpp;App/Model/FromRoot.cpp;App/Model/FromHere.cpp"
    "App/Model/FromRoot.cpp: #include" "App/Model/FromHere.cpp: #include")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -P "${CHECK_SCRIPT}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
)
math(EXPR case_number "${case_number} + 1")
if(result EQUAL 0 OR NOT output MATCHES "pass -DCONFIG=<file>")
    set(failures "${failures}\nThe check without a configuration must fail asking for one; it printed:\n${output}")
endif()

if(failures)
    message(FATAL_ERROR "TestModelIncludeCheck failed:${failures}")
endif()
message(STATUS "TestModelIncludeCheck: ${case_number} cases passed")
