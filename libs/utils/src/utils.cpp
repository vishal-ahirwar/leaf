#include "../include/utils.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

namespace Utils
{

bool replaceString(std::string& str, const std::string& from, const std::string& to)
{
    if (str.empty())
    {
        return false;
    };
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return true;
}

std::vector<std::string> betterArgs(size_t argc, char** argv)
{
    std::vector<std::string> args{};
    for (size_t i = 0; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    return args;
}

void printLeafConfigPath()
{
#ifdef _WIN32
    auto appdata = std::getenv("APPDATA");
    if (appdata)
    {
        std::cout << "Leaf config: " << appdata << "\\.leaf" << std::endl;
    }
#elif defined(__APPLE__)
    auto home = std::getenv("HOME");
    if (home)
    {
        std::cout << "Leaf config: " << home << "/Library/Application Support/.leaf" << std::endl;
    }
#else
    auto xdg  = std::getenv("XDG_CONFIG_HOME");
    auto home = std::getenv("HOME");
    if (xdg)
    {
        std::cout << "Leaf config: " << xdg << "/.leaf" << std::endl;
    }
    else if (home)
    {
        std::cout << "Leaf config: " << home << "/.config/.leaf" << std::endl;
    }
#endif
}

int generateProfiles()
{
    // Handled by CLI::generateProfile() — this is a legacy stub
    return 0;
}

std::string getOSProfilePath()
{
    std::string os_profile;
#ifdef _WIN32
    os_profile = "profiles/windows_profile";
#elif defined(__APPLE__)
    os_profile = "profiles/macos_profile";
#else
    os_profile = "profiles/linux_profile";
#endif
    return os_profile;
}

std::string trim(const std::string& s)
{
    auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

bool isValidProjectName(const std::string& name)
{
    if (name.empty())
        return false;
    for (char c : name)
    {
        if (std::isspace(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}

} // namespace Utils
