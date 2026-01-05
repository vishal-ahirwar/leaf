#Auto Generated Root CMake file by Sage
#Copyright(c) 2025 None.All rights reerved.
cmake_minimum_required(VERSION 3.6...3.31)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(tools.config)
project(leaf VERSION 0.0.9 LANGUAGES CXX C)
include(app.config)
if(BUILD_APPLICATION)
    add_subdirectory(external/spinner)
    add_subdirectory(apps)
    add_subdirectory(tests)
endif(BUILD_APPLICATION)
add_subdirectory(libs)
