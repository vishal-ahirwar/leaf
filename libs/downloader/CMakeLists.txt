add_library(downloader src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
find_package(fmt)
find_package(cpr)
target_link_libraries(downloader fmt::fmt cpr::cpr)