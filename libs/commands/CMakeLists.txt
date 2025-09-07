
add_library(commands src/commands.cpp)
target_include_directories(commands PUBLIC include)
target_link_libraries(commands fmt::fmt)
install(TARGETS commands)
install(DIRECTORY include/ DESTINATION include)