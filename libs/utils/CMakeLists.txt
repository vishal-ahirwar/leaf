add_library(utils src/utils.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_include_directories(utils PUBLIC include)

install(TARGETS utils)
install(DIRECTORY include/ DESTINATION include/utils)
