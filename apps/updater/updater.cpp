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
    Leaf::Logger::log("Would You like to check for update and install latest version of leaf?");
    std::string input{};
    std::getline(std::cin, input);
    if (!input.empty() && input == "yes")
    {
        Downloader::download("https://github.com/vishal-ahirwar/flick-installer/releases/download/"
                             "flick-installer-1.1.0/FlickInstaller.exe",
                             "flickinstaller.exe");
    }
    return 0;
}
