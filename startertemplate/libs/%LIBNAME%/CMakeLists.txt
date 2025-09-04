
add_library(%LIBNAME% src/%LIBNAME%.cpp)

target_include_directories(%LIBNAME% PUBLIC include)

install(TARGETS %LIBNAME%
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

