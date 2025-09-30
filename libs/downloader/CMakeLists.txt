add_library(downloader src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(downloader PRIVATE fmt::fmt cpr::cpr nlohmann_json::nlohmann_json)
target_include_directories(downloader PUBLIC include)
install(TARGETS downloader)
install(DIRECTORY include/ DESTINATION include/downloader)
