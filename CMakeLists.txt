#Auto Generated Root CMake file by Sage
#Copyright(c) 2025 None.All rights reerved.
cmake_minimum_required(VERSION 3.6...3.31)

include(cmake/clang.config.cmake)
project(leaf VERSION 0.1.0 LANGUAGES CXX C)
include(cmake/config.cmake)

#@add_find_package Warning: Do not remove this line

#@add_subproject Warning: Do not remove this line

if(BUILD_APPLICATION)
    add_subdirectory(external/spinner)
    add_subdirectory(apps)
    add_subdirectory(tests)
endif(BUILD_APPLICATION)

add_subdirectory(libs)
