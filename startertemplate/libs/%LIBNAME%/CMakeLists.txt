add_library(%LIBNAME% src/lib.cpp)

target_include_directories(%LIBNAME% PUBLIC include)
target_link_libraries(%LIBNAME% PUBLIC fmt::fmt)

install(TARGETS %LIBNAME%
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

