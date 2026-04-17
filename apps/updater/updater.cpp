#include <downloader.h>
#include <leafconfig.h>
#include <logger.h>
#include <utils.h>

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    Leaf::Logger::log(std::string("Updater v") + Project::VERSION_STRING.data() + " " +
                      Project::PROJECT_NAME.data());
    Leaf::Logger::log(
        "Would you like to check for updates and install the latest version of leaf?");
    std::string input{};
    std::getline(std::cin, input);
    if (!input.empty() && (input == "yes" || input == "y"))
    {
#ifdef _WIN32
        Downloader::download(
            "https://github.com/vishal-ahirwar/leaf/releases/latest/download/leaf-windows.zip",
            "leaf-update.zip");
#elif defined(__APPLE__)
        Downloader::download(
            "https://github.com/vishal-ahirwar/leaf/releases/latest/download/leaf-macos.zip",
            "leaf-update.zip");
#else
        Downloader::download(
            "https://github.com/vishal-ahirwar/leaf/releases/latest/download/leaf-linux.zip",
            "leaf-update.zip");
#endif
        Leaf::Logger::info(
            "Downloaded update. Please extract and replace the existing leaf binary.");
    }
    return 0;
}
