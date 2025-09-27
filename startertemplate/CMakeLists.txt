cmake_minimum_required(VERSION 3.15)
include(cmake/clang.config.cmake)
project(%APPNAME% VERSION 0.1.0 LANGUAGES CXX)
include(cmake/config.cmake)

if(BUILD_APPLICATION)
    add_subdirectory(src)
endif()

add_subdirectory(libs)
