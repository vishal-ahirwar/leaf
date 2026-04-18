
find_package(httplib REQUIRED)
add_library(server src/server.cpp)

include(FetchContent)
FetchContent_Declare(cpp-embedlib
    GIT_REPOSITORY https://github.com/yhirose/cpp-embedlib
    GIT_TAG        main
)
FetchContent_MakeAvailable(cpp-embedlib)

cpp_embedlib_add(WebAssets
    FOLDER    ${CMAKE_SOURCE_DIR}/server/public
    NAMESPACE Web
)

target_include_directories(server PUBLIC include)

target_compile_features(server PUBLIC cxx_std_20)
target_link_libraries(server PRIVATE httplib::httplib WebAssets cpp-embedlib-httplib)
install(TARGETS server)
install(DIRECTORY include/ DESTINATION include/server)
