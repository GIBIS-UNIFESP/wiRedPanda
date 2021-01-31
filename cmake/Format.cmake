include(KDEClangFormat)

file(
    GLOB_RECURSE
    ALL_CLANG_FORMAT_SOURCE_FILES
        *.cpp *.h
    ${PROJECT_SOURCE_DIR}
)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})