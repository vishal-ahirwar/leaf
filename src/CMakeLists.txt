find_package(fmt)
find_package(reproc++)
find_package(spinner)
find_package(platform_folders)

add_executable(leaf leaf/leaf.cpp leaf/leafcommands.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(leaf  utils generator downloader spinner::spinner easyproc commands sago::platform_folders)

add_executable(updater updater/updater.cpp)
target_link_libraries(updater downloader)
