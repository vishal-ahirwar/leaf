#include "leafcommands.h"

#include <downloder.h>
#include <easyproc.h>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <sago/platform_folders.h>
#include <utils.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "leafconfig.h"

LeafCommands::LeafCommands(std::vector<std::string>&& args)
    : _commands(new Commands(std::move(args),
                             []()
                             {
                                 fmt::print("🍃 Leaf - A modern C++ project manager.\n");
                                 fmt::print("Run 'leaf help' for a list of commands.\n");
                             }))
{
    _commands->registerCommands(
        "create",
        "Generates a new, fully structured Leaf project in a new directory.",
        [this]() { return this->create(); });

    _commands->registerCommands("help",
                                "Displays a list of all available commands and their descriptions.",
                                [this]() { return this->help(); });
    _commands->registerCommands(
        "version", "Display leaf current version", [this]() { return this->version(); });
    _commands->registerCommands(
        "clean",
        "Deletes all build files and temporary artifacts from the project directory.",
        [this]() { return this->clean(); });
    _commands->registerCommands(
        "format",
        "Automatically formats all C++ source code files according to a consistent style.",
        [this]() { return this->format(); });
    _commands->registerCommands(
        "install",
        "Fetches and installs all the dependencies listed in the conanfile.py.",
        [this]() { return this->install(); });
    _commands->registerCommands("build",
                                "A comprehensive command to compile the entire project and its "
                                "dependencies and run the app.",
                                [this]() { return this->build(); });
    _commands->registerCommands("compile",
                                "Builds all targets in the project without running them.",
                                [this]() { return this->compile(); });
    _commands->registerCommands(
        "run",
        "Compiles and executes the main application target of your project.",
        [this]() { return this->run(); });
    _commands->registerCommands(
        "publish",
        "Formally uploads the final, release-ready version of a package to a remote.",
        [this]() { return this->publish(); });
    _commands->registerCommands(
        "upload",
        "Uploads a packaged library to a specified remote Conan repository.",
        [this]() { return this->upload(); });
    _commands->registerCommands(
        "doctor",
        "Checks your system to ensure all required tools (Clang, CMake,Conan) ",
        [this]() { return this->doctor(); });
    _commands->registerCommands(
        "release",
        "Fetches and installs in release mode all the dependencies listed in the conanfile.py.",
        [this]() { return this->release(); });
    _commands->registerCommands(
        "init",
        "Initializes a new Leaf project structure within an existing directory.",
        [this]() { return this->init(); });
};

int LeafCommands::install()
{
    std::vector<std::string> conanInstallArgs{"conan",
                                              "install",
                                              ".",
                                              "-of",
                                              ".install",
                                              "-b",
                                              "missing",
                                              "-c",
                                              "tools.cmake.cmaketoolchain:user_presets=",
                                              "-o",
                                              "&:build_app=True"};
    conanInstallArgs.push_back("-s");
    if (std::ranges::find(_commands->getArgs(), "-r") != _commands->getArgs().end())
    {
        conanInstallArgs.push_back("build_type=Release");
    }
    else
    {
        conanInstallArgs.push_back("build_type=Debug");
    }
    runExternalProcess(conanInstallArgs);
    return 0;
};

int LeafCommands::create()
{
    namespace fs = std::filesystem;

    std::string lib_name;
    std::string project_name{};

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your project? ");

    std::getline(std::cin, project_name);
    if (project_name.find(" ") != std::string::npos || project_name.empty())
    {
        fmt::println("Project name can't be empty and can't have whitespaces in their name!");
        return 0;
    }
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "lib name? ");

    std::getline(std::cin, lib_name);
    if (project_name == lib_name)
    {
        fmt::println("You can't have two targets with same name!");
        return 0;
    };
    lib_name = lib_name.empty() ? project_name + "lib" : lib_name;

    std::map<std::string, std::string> replacements{
        {"%APPNAME%", project_name}, {"%LIBNAME%", lib_name}, {"startertemplate", project_name}};
    auto home = sago::getConfigHome();
    if (fs::exists(home))
    {
        auto starter_template = fs::path(home) / ".leaf" / "startertemplate";
        if (!fs::exists(starter_template))
        {
            fs::create_directories(starter_template);
            downloadGithubDirectory(
                "vishal-ahirwar", "leaf", "startertemplate", starter_template.string());
        }
        if (!fs::exists(starter_template))
        {
            fmt::println("Could not find or download start template code!");
            return 1;
        };
        if (!fs::exists(fs::current_path() / project_name))
        {
            fs::create_directories(fs::current_path() / project_name);
        }

        for (const auto& templateContent : fs::recursive_directory_iterator(starter_template))
        {
            auto oldPath = templateContent.path().string();
            fs::path newPath{};
            auto index = oldPath.find("startertemplate");
            if (index != std::string::npos)
            {
              newPath = (fs::current_path() / oldPath.substr(index));
            }
            std::string name  = newPath.filename().string();
            std::string newPathString = newPath.string();
            std::ranges::for_each(replacements,
                                  [&newPathString](const auto& rep)
                                  { replaceString(newPathString, rep.first, rep.second); });
            if (templateContent.is_directory())
            {
                if (fs::create_directories(newPathString))
                {
                    fmt::print("Directory created :{}\n", newPathString);
                }
            }
            else
            {
                fmt::print("File created:{}\n", newPathString);

                std::ifstream in(oldPath);
                std::string   content{};
                in >> std::noskipws;

                std::ranges::copy(std::istream_iterator<char>(in), std::istream_iterator<char>{},std::back_inserter(content));
                std::ranges::for_each(replacements,
                                      [&content](const auto& rep) { replaceString(content,rep.first,rep.second); });
                std::ofstream out(newPathString);
                out << content;
                out.close();
            }
        }
    }
    else
    {
        fmt::print("{} does not exist!\n", home);
        return -1;
    }

    return 0;
};

int LeafCommands::publish()
{
    runExternalProcess({"conan", "create", "."});
    return 0;
};

int LeafCommands::upload()
{
    // TODO
    return 0;
};

int LeafCommands::runTests()
{
    runExternalProcess({"ctest -B .build/Debug/tests"});
    return 0;
};

int LeafCommands::format()
{

    return 0;
};

int LeafCommands::clean()
{
    runExternalProcess({"cmake", "--preset", "debug", "--fresh"});
    return 0;
};

int LeafCommands::release()
{
    std::vector<std::string> conanInstallArgs{"conan",
                                              "install",
                                              ".",
                                              "-of",
                                              ".install",
                                              "-b",
                                              "missing",
                                              "-s",
                                              "build_type=Release",
                                              "-c",
                                              "tools.cmake.cmaketoolchain:user_presets=",
                                              "-o",
                                              "&:build_app=True"};
    runExternalProcess(conanInstallArgs);
    runExternalProcess({"cmake", "--preset", "release", "--fresh"});
    runExternalProcess({"cmake", "--build", "--preset", "release"});
    return 0;
};

int LeafCommands::addPackage()
{
    return 0;
};

int LeafCommands::removePackage()
{
    return 0;
};

int LeafCommands::addApp()
{
    return 0;
};

int LeafCommands::addLib()
{
    return 0;
};

int LeafCommands::doctor()
{
    return 0;
};

int LeafCommands::build()
{
    namespace fs = std::filesystem;
    if (!fs::exists(".install"))
    {
        install();
    };
    if (!fs::exists(".build"))
    {
        runExternalProcess({"cmake", "--preset", "debug"});
    }

    runExternalProcess({"cmake", "--build", "--preset", "debug"});
    return 0;
}

int LeafCommands::compile()
{
    return 0;
}

int LeafCommands::run()
{
    namespace fs = std::filesystem;
#ifdef _WIN322
    std::string extention = ".exe";
#else
    std::string extention = "";
#endif
    if (std::ranges::find(_commands->getArgs(), "-r") != _commands->getArgs().end())
    {
        auto appName = _commands->getArgs().back();
        if (appName == "-r" || _commands->getArgs().size() <= 2)
        {
            appName = fs::current_path().filename().string();
        }
        std::system(
            fmt::format("./.build/Release/apps/{}/{}{}", appName, appName, extention).c_str());
    }
    else
    {
        auto appName = _commands->getArgs().back();
        if (_commands->getArgs().size() <= 2)
        {
            appName = fs::current_path().filename().string();
        }
        std::system(
            fmt::format("./.build/Debug/apps/{}/{}{}", appName, appName, extention).c_str());
    }
    return 0;
}

int LeafCommands::init()
{
    return 0;
}

int LeafCommands::help()
{
    fmt::print("\n🍃 Leaf - v{} by {}\n", Project::VERSION_STRING, Project::COMPANY_NAME);
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "A modern, fast, and intuitive project/package manager for C++\n\n");
    std::map<std::string, std::pair<std::string, std::function<int()>>> sorted_commands{
        _commands->getCommands().begin(), _commands->getCommands().end()};
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "Available Commands\n");
    std::ranges::for_each(sorted_commands,
                          [](const auto& command)
                          {
                              fmt::print("{} - ", command.first);
                              fmt::print(fmt::emphasis::faint | fmt::fg(fmt::color::floral_white),
                                         "{}\n",
                                         command.second.first);
                          });
    fmt::println("");
    return 0;
}

int LeafCommands::version() const
{
    fmt::println("{} v{}", Project::PROJECT_NAME, Project::VERSION_STRING);
    return 0;
}

int LeafCommands::exec()
{
    return _commands->exec();
}