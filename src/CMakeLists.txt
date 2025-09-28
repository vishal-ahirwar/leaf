find_package(fmt)

add_executable(leaf leaf.cpp) # Add your Source Files here
target_link_libraries(leaf commands utils)


add_executable(updater updater.cpp)
target_link_libraries(updater downloader)
