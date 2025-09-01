add_library(generator src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
find_package(fmt)
target_link_libraries(generator fmt::fmt)