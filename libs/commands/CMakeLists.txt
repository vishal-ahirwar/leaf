
add_library(commands src/commands.cpp
        src/LeafCommands.cpp)
target_include_directories(commands PUBLIC include)

target_link_libraries(commands PRIVATE fmt::fmt downloader easyproc utils sago::platform_folders spinner logger)

install(TARGETS commands)
install(DIRECTORY include/ DESTINATION include/commands)