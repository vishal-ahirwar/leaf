add_library(%LIBNAME% %LIBNAME%/src/%LIBNAME%.cpp)

target_include_directories(%LIBNAME% PUBLIC %LIBNAME%/include)

install(TARGETS %LIBNAME%)
install(DIRECTORY %LIBNAME%/include/ DESTINATION include/%LIBNAME%)


