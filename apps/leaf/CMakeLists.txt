add_executable(leaf src/main.cpp src/leafcommands.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(leaf fmt::fmt utils generator downloader spinner::spinner easyproc commands)