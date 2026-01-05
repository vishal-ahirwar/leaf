find_package(fmt)
add_library(logger src/logger.cpp)

target_include_directories(logger PUBLIC include)
target_link_libraries(logger PRIVATE fmt::fmt)

install(TARGETS logger)
install(DIRECTORY include/ DESTINATION include/logger)
