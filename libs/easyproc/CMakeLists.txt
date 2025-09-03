
find_package(reproc++)
find_package(fmt)

add_library(easyproc src/main.cpp)

target_include_directories(easyproc PUBLIC include)
target_link_libraries(easyproc PRIVATE reproc::reproc fmt::fmt)

install(TARGETS easyproc
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)