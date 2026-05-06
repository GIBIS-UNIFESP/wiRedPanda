# CMake toolchain file for cross-compiling to ARM64 on Linux

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify cross-compiler
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Search for programs in cross-tools
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

# Search for libraries and includes in cross-tools
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Set C++ standard
string(APPEND CMAKE_CXX_FLAGS_INIT " -march=armv8-a")
