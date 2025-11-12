#include <downloder.h>
#include <leafconfig.h>
#include <logger.h>
#include <utils.h>

#include <iostream>
#include <string>


int main(int argc, char** argv)
{
    Logger::Logger::log(std::string("Updater v") + Project::VERSION_STRING.data() + " " +
                        Project::PROJECT_NAME.data());
    Logger::Logger::log("Would You like to check for update and install latest version of leaf?");
    std::string input{};
    std::getline(std::cin, input);
    if (!input.empty() && input == "yes")
    {
        Downloder::download("https://github.com/vishal-ahirwar/flick-installer/releases/download/"
                            "flick-installer-1.1.0/FlickInstaller.exe",
                            "flickinstaller.exe");
    }
    return 0;
}
