cmake_minimum_required(VERSION 3.15)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(tools.config)
project(%APPNAME% VERSION 0.1.0 LANGUAGES CXX)
include(app.config)

if(BUILD_APPLICATION)
    add_subdirectory(apps)
endif()

add_subdirectory(libs)
