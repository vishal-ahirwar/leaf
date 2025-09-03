cmake_minimum_required(VERSION 3.15)
project(%PROJECT_NAME% LANGUAGES CXX)

if(BUILD_APPLICATION)
    add_subdirectory(apps)  
endif(BUILD_APPLICATION)

add_subdirectory(libs)