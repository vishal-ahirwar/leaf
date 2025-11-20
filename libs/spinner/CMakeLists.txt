add_library(spinner src/spinner.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(spinner PRIVATE fmt::fmt)
target_include_directories(spinner PUBLIC include)
install(TARGETS spinner)
install(DIRECTORY include/ DESTINATION include)