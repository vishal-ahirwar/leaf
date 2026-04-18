find_package(fmt REQUIRED)
find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(platform_folders REQUIRED)
find_package(pystring REQUIRED)

add_library(commands
        src/commands.cpp
        src/commandregistry.cpp
        src/build_commands.cpp
        src/package_commands.cpp
        src/project_commands.cpp
        src/toolchain_commands.cpp
        src/search_commands.cpp
        src/server_commands.cpp
) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line

target_link_libraries(commands PRIVATE utils downloader logger generator easyproc fmt::fmt sago::platform_folders spinner pystring::pystring server)
target_include_directories(commands PUBLIC include ${CMAKE_SOURCE_DIR}/external/cargs/include)
install(TARGETS commands)
install(DIRECTORY include/ DESTINATION include/commands)
