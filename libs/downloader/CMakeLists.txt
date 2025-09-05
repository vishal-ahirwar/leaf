add_library(downloader src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(downloader fmt::fmt cpr::cpr)
target_include_directories(downloader PUBLIC include)
install(TARGETS downloader
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)