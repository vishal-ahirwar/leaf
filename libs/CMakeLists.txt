find_package(fmt)
find_package(reproc++)
find_package(cpr)
find_package(nlohmann_json)

add_library(downloader downloader/src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(downloader fmt::fmt cpr::cpr nlohmann_json::nlohmann_json)
target_include_directories(downloader PUBLIC include)
install(TARGETS downloader)
install(DIRECTORY include/ DESTINATION include)

add_library(easyproc easyproc/src/main.cpp)
target_include_directories(easyproc PUBLIC include)
target_link_libraries(easyproc PRIVATE reproc::reproc fmt::fmt)

install(TARGETS easyproc)
install(DIRECTORY include/ DESTINATION include)

add_library(generator generator/src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(generator fmt::fmt)
target_include_directories(generator PUBLIC include)
install(TARGETS generator)
install(DIRECTORY include/ DESTINATION include)


add_library(utils utils/src/utils.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_include_directories(utils PUBLIC include)

install(TARGETS utils)
install(DIRECTORY include/ DESTINATION include)

add_library(commands commands/src/commands.cpp)
target_include_directories(commands PUBLIC include)
target_link_libraries(commands fmt::fmt)
install(TARGETS commands)
install(DIRECTORY include/ DESTINATION include)