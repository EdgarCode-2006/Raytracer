# Cross-compiles the ray tracer for 64-bit Windows from Linux (or WSL) with MinGW-w64.
#
#   sudo apt install mingw-w64
#   cmake -B build-windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake
#   cmake --build build-windows -j
#
# Produces build-windows/client.exe and rayFeatures/*.dll.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(MINGW_TRIPLET x86_64-w64-mingw32)

# Debian/Ubuntu ship a "-posix" and a "-win32" flavour of the compilers,
# the posix one has the complete std::thread / std::mutex support.
find_program(CMAKE_C_COMPILER NAMES ${MINGW_TRIPLET}-gcc-posix ${MINGW_TRIPLET}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${MINGW_TRIPLET}-g++-posix ${MINGW_TRIPLET}-g++)
find_program(CMAKE_RC_COMPILER NAMES ${MINGW_TRIPLET}-windres)

# Look for Windows headers and libraries in the MinGW sysroot only, never in the Linux ones.
set(CMAKE_FIND_ROOT_PATH /usr/${MINGW_TRIPLET})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
