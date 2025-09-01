add_executable(leaf src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
find_package(fmt)
find_package(reproc++)
target_link_libraries(leaf fmt::fmt reproc::reproc utils generator downloader)