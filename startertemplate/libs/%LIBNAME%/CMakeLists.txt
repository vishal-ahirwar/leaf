add_library(%LIBNAME% src/%LIBNAME%.cpp)

target_include_directories(%LIBNAME% PUBLIC include)

install(TARGETS %LIBNAME%)
install(DIRECTORY include/ DESTINATION include/%LIBNAME%)
