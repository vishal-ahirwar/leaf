//
// Project scaffolding: create, init, addApp, addLib
// Refactored to share a common template engine.
//

#include "commands.h"

#include <downloader.h>
#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <sago/platform_folders.h>
#include <utils.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <string>

#include "logger.h"

namespace Leaf
{

namespace
{


int ensureTemplateExists(std::filesystem::path& out_path)
{
    namespace fs = std::filesystem;
    auto home    = sago::getConfigHome();
    if (!fs::exists(home))
    {
        Leaf::Logger::error(fmt::format("{} does not exist!", home));
        return -1;
    }
    out_path = fs::path(home) / ".leaf" / "startertemplate";
    if (!fs::exists(out_path))
    {
        fs::create_directories(out_path);
        Downloader::downloadGithubDirectory(
            "vishal-ahirwar", "leaf", "startertemplate", out_path.string());
    }
    if (!fs::exists(out_path))
    {
        Leaf::Logger::error("Could not find or download starter template code!");
        return 1;
    }
    return 0;
}

void expandTemplate(const std::filesystem::path&              template_dir,
                    const std::filesystem::path&              dest_dir,
                    const std::map<std::string, std::string>& replacements,
                    const std::string&                        path_filter  = "",
                    bool                                      skip_existing = false)
{
    namespace fs = std::filesystem;
    for (const auto& entry : fs::recursive_directory_iterator(template_dir))
    {
        auto     oldPath = entry.path().string();
        fs::path newPath{};
        auto     index = oldPath.find("startertemplate");
        if (index == std::string::npos)
            continue;

        if (!path_filter.empty())
        {
            if (oldPath.find(path_filter) == std::string::npos)
                continue;
            auto filterIdx = oldPath.find(path_filter);
            newPath        = dest_dir / oldPath.substr(filterIdx);
        }
        else
        {
            newPath = dest_dir / oldPath.substr(index);
        }

        std::string newPathString = newPath.string();
        for (const auto& [from, to] : replacements)
            Utils::replaceString(newPathString, from, to);

        if (skip_existing && fs::exists(newPathString))
        {
            fmt::println("Skipping existing: {}", newPathString);
            continue;
        }

        if (entry.is_directory())
        {
            if (fs::create_directories(newPathString))
                fmt::print("Directory created: {}\n", newPathString);
        }
        else
        {
            fmt::print("File created: {}\n", newPathString);
            std::ifstream in(oldPath);
            std::string   content{};
            in >> std::noskipws;
            std::ranges::copy(std::istreambuf_iterator<char>(in),
                              std::istreambuf_iterator<char>{},
                              std::back_inserter(content));
            for (const auto& [from, to] : replacements)
                Utils::replaceString(content, from, to);
            std::ofstream out(newPathString);
            out << content;
        }
    }
}

} // namespace


int CLI::create()
{
    namespace fs = std::filesystem;

    std::string lib_name;
    std::string project_name{};

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your project? ");

    std::getline(std::cin, project_name);
    if (!Utils::isValidProjectName(project_name))
    {
        Leaf::Logger::error("Project name can't be empty or contain whitespaces!");
        return 1;
    }
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "lib name? ");

    std::getline(std::cin, lib_name);
    if (project_name == lib_name)
    {
        Leaf::Logger::error("You can't have two targets with the same name!");
        return 1;
    }
    lib_name = lib_name.empty() ? project_name + "lib" : lib_name;

    std::map<std::string, std::string> replacements{{"%APPNAME%", project_name},
                                                    {"%LIBNAME%", lib_name},
                                                    {"startertemplate", project_name},
                                                    {"%WORKSPACE%", project_name}};

    fs::path template_dir;
    if (int rc = ensureTemplateExists(template_dir); rc != 0)
        return rc;

    if (!fs::exists(fs::current_path() / project_name))
        fs::create_directories(fs::current_path() / project_name);

    expandTemplate(template_dir, fs::current_path(), replacements);
    Leaf::Logger::success(fmt::format("Project '{}' created successfully!", project_name));
    return 0;
}


int CLI::init()
{
    namespace fs             = std::filesystem;
    std::string project_name = fs::current_path().filename().string();
    std::string lib_name     = project_name + "lib";

    if (!_commands->getPositionals().empty())
    {
        project_name = _commands->getPositionals().front();
        lib_name     = project_name + "lib";
    }

    fmt::println("Initializing project '{}' in current directory.", project_name);

    std::map<std::string, std::string> replacements{{"%APPNAME%", project_name},
                                                    {"%LIBNAME%", lib_name},
                                                    {"startertemplate", project_name},
                                                    {"%WORKSPACE%", project_name}};

    fs::path template_dir;
    if (int rc = ensureTemplateExists(template_dir); rc != 0)
        return rc;

    // For init we expand into the current directory, skipping anything that exists
    for (const auto& entry : fs::recursive_directory_iterator(template_dir))
    {
        auto oldPath = entry.path().string();
        auto index   = oldPath.find("startertemplate");
        if (index == std::string::npos)
            continue;

        std::string relPath = oldPath.substr(index + std::string("startertemplate").length());
        if (!relPath.empty() && (relPath[0] == '/' || relPath[0] == '\\'))
            relPath = relPath.substr(1);
        if (relPath.empty())
            continue;

        fs::path    newPath       = fs::current_path() / relPath;
        std::string newPathString = newPath.string();
        for (const auto& [from, to] : replacements)
            Utils::replaceString(newPathString, from, to);

        if (fs::exists(newPathString))
        {
            fmt::println("Skipping existing: {}", newPathString);
            continue;
        }

        if (entry.is_directory())
        {
            if (fs::create_directories(newPathString))
                fmt::print("Directory created: {}\n", newPathString);
        }
        else
        {
            fmt::print("File created: {}\n", newPathString);
            std::ifstream in(oldPath);
            std::string   content{};
            in >> std::noskipws;
            std::ranges::copy(std::istreambuf_iterator<char>(in),
                              std::istreambuf_iterator<char>{},
                              std::back_inserter(content));
            for (const auto& [from, to] : replacements)
                Utils::replaceString(content, from, to);
            std::ofstream out(newPathString);
            out << content;
        }
    }

    Leaf::Logger::success(fmt::format("Project '{}' initialized.", project_name));
    return 0;
}


int CLI::addApp()
{
    namespace fs = std::filesystem;

    std::string project_name{};
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your app? ");

    std::getline(std::cin, project_name);
    if (!Utils::isValidProjectName(project_name))
    {
        Leaf::Logger::error("App name can't be empty or contain whitespaces!");
        return 1;
    }

    std::map<std::string, std::string> replacements{
        {"%APPNAME%", project_name}, {"%WORKSPACE%", fs::current_path().filename().string()}};

    fs::path template_dir;
    if (int rc = ensureTemplateExists(template_dir); rc != 0)
        return rc;

    expandTemplate(template_dir, fs::current_path() / "apps", replacements, "%APPNAME%");

    std::ofstream cmake("apps/CMakeLists.txt", std::ios::app);
    if (cmake.is_open())
        cmake << "add_subdirectory(" << project_name << ")\n";
    cmake.close();

    Leaf::Logger::success(fmt::format("App '{}' added.", project_name));
    return 0;
}

int CLI::addLib()
{
    namespace fs = std::filesystem;

    std::string project_name{};
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your lib? ");

    std::getline(std::cin, project_name);
    if (!Utils::isValidProjectName(project_name))
    {
        Leaf::Logger::error("Lib name can't be empty or contain whitespaces!");
        return 1;
    }

    std::map<std::string, std::string> replacements{
        {"%LIBNAME%", project_name}, {"%WORKSPACE%", fs::current_path().filename().string()}};

    fs::path template_dir;
    if (int rc = ensureTemplateExists(template_dir); rc != 0)
        return rc;

    expandTemplate(template_dir, fs::current_path() / "libs", replacements, "%LIBNAME%");

    std::ofstream cmake{"libs/CMakeLists.txt", std::ios::app};
    if (cmake.is_open())
    {
        cmake << "add_subdirectory(" << project_name << ")\n";
        cmake.close();
    }

    Leaf::Logger::success(fmt::format("Library '{}' added.", project_name));
    return 0;
}

} // namespace Leaf
