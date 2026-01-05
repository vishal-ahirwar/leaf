
find_package(platform_folders REQUIRED)
add_executable(leaf leaf.cpp commands.cpp) # Add your Source Files here
target_include_directories(leaf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(leaf utils downloader logger generator easyproc fmt::fmt sago::platform_folders spinner)
