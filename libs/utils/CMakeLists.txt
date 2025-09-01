add_library(utils src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
find_package(fmt)
find_package(reproc++)
target_link_libraries(utils fmt::fmt reproc::reproc)