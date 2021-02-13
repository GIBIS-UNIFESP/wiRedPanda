include(KDEClangFormat)

file(
    GLOB_RECURSE
    ALL_CLANG_FORMAT_SOURCE_FILES
    RELATIVE
    ${PROJECT_SOURCE_DIR}
    *.cpp *.h
)

kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
