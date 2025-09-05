add_library(generator src/main.cpp) # Add your Source Files here
#@add_target_link_libraries Warning: Do not remove this line
target_link_libraries(generator fmt::fmt)
target_include_directories(generator PUBLIC include)
install(TARGETS generator
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
