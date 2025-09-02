find_package(fmt)
find_package(reproc++)
find_package(spinner)
add_library(utils src/utils.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_include_directories(utils PUBLIC include)
target_link_libraries(utils fmt::fmt reproc::reproc spinner::spinner)