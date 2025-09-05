
add_library(commands src/commands.cpp)
target_include_directories(commands PUBLIC include)
target_link_libraries(commands fmt::fmt)
install(TARGETS commands
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)