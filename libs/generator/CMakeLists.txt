find_package(fmt)
add_library(generator src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(generator PRIVATE fmt::fmt)
target_include_directories(generator PUBLIC include)
install(TARGETS generator)
install(DIRECTORY include/ DESTINATION include/generator)
