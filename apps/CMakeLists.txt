find_package(fmt)
find_package(reproc++)
find_package(spinner)
find_package(platform_folders)

add_executable(leaf leaf/src/main.cpp leaf/src/leafcommands.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(leaf fmt::fmt utils generator downloader spinner::spinner easyproc commands sago::platform_folders)

add_executable(updater updater/src/main.cpp)
target_link_libraries(updater downloader)
