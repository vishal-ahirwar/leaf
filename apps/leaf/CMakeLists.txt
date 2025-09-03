add_executable(leaf src/main.cpp src/commands.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
find_package(fmt)
find_package(reproc++)
find_package(spinner)
target_link_libraries(leaf fmt::fmt utils generator downloader spinner::spinner easyproc)