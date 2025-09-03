if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Show a message for clarity
message(STATUS "Forcing Clang compiler")

# Detect host system
if(NOT DEFINED CMAKE_SYSTEM_NAME)
    set(CMAKE_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})
endif()

# Force Ninja generator if not already set
if(NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
    message(STATUS "Warning: CMake Generator: ${CMAKE_GENERATOR}, Use Ninja for faster builds")
else()
    message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")
endif()

# Windows: use clang-cl
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    message(STATUS "Targeting Windows — using clang-cl")
    # You can customize this path if needed
    set(CMAKE_C_COMPILER "clang-cl" CACHE STRING "C compiler")
    set(CMAKE_CXX_COMPILER "clang-cl" CACHE STRING "C++ compiler")

    # Optional: specify MSVC-like flags
    set(CMAKE_C_FLAGS "/nologo /EHsc" CACHE STRING "C flags")
    set(CMAKE_CXX_FLAGS "/nologo /EHsc" CACHE STRING "C++ flags")

# Android: use NDK toolchain
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    message(STATUS "Targeting Android — using NDK Clang")

    set(CMAKE_ANDROID_NDK "/path/to/ndk" CACHE PATH "Path to Android NDK")
    set(CMAKE_SYSTEM_VERSION 21)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)

    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake")

# Linux/macOS: use regular clang
else()
    message(STATUS "Targeting Unix-like system — using clang/clang++")

    set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler")
    set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler")
endif()

