find_package(fmt)
find_package(cpr)
find_package(nlohmann_json)
find_package(platform_folders REQUIRED)
find_package(pystring)
add_library(commands  src/commands.cpp
        src/CommandRegistry.cpp
) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(commands PRIVATE utils downloader logger generator easyproc fmt::fmt sago::platform_folders spinner pystring::pystring)
target_include_directories(commands PUBLIC include)
install(TARGETS commands)
install(DIRECTORY include/ DESTINATION include/commands)
