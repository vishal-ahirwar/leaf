find_package(fmt)
add_library(%LIBNAME%lib src/%LIBNAME%.cpp)
target_link_libraries(%LIBNAME%lib fmt::fmt)
target_include_directories(%LIBNAME%lib PUBLIC include)

install(TARGETS %LIBNAME%lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

