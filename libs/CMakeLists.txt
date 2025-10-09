find_package(fmt)
find_package(reproc++)
find_package(cpr)
find_package(nlohmann_json)
find_package(platform_folders)
find_package(spinner)


add_subdirectory(easyproc)
add_subdirectory(utils)
add_subdirectory(generator)
add_subdirectory(commands)
add_subdirectory(downloader)
add_subdirectory(logger)
