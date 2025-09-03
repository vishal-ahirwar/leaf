#Auto Generated Root CMake file by Sage
#None
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CRT_STATIC_LINK ON)
option(BUILD_APPLICATION ON)

message(STATUS "Static Link C Runtime ${CRT_STATIC_LINK}")
message(STATUS "Build Application ${BUILD_APPLICATION}")

if(CRT_STATIC_LINK)
  if (WIN32)
      set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
  endif()
else()
  if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
  endif()
endif()

set(COMPANY "Vishal Ahirwar")
string(TIMESTAMP CURRENT_YEAR "%Y")
set(COPYRIGHT "Copyright(c) ${CURRENT_YEAR} ${COMPANY}.")

include_directories(${CMAKE_BINARY_DIR})
configure_file(cmake/leafconfig.h.in leafconfig.h)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Enabling secure coding features for Clang")
    add_compile_options(
        -Xclang
        -Wall -Wextra -Wpedantic
        -Wshadow -Wold-style-cast
        -Wcast-align -Wnull-dereference
        -Wformat=2 -Wformat-security
        -D_FORTIFY_SOURCE=2
        -Werror
    )
endif()
