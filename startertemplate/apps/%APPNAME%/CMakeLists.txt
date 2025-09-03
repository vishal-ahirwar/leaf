add_executable(%APPNAME% src/main.cpp)

target_link_libraries(%APPNAME% PRIVATE %LIBNAME%lib)
