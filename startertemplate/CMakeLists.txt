cmake_minimum_required(VERSION 3.15)
include(cmake/clang.toolchain.cmake)
project(%APPNAME% LANGUAGES CXX)
include(cmake/config.cmake)

if(BUILD_APPLICATION)
    add_subdirectory(apps)
endif()

add_subdirectory(libs)
