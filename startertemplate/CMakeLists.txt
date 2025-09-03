cmake_minimum_required(VERSION 3.15)
project(%APPNAME% LANGUAGES CXX)

if(BUILD_APPLICATION)
    add_subdirectory(apps)
endif()

add_subdirectory(libs)
