//
// Created by itsvi on 10/22/2025.
//

#include "../include/commands.h"

#include <downloder.h>
#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <leafconfig.h>
#include <sago/platform_folders.h>
#include <spinner.h>
#include <utils.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

#include "logger.h"

namespace Commands
{
int Commands::exec()
{
    if (_args.size() < 2)
    {
        _default();
        return 1;
    }
    bool commandExists{false};
    std::for_each(_args.begin() + 1,
                  _args.end(),
                  [this, &commandExists](const auto& command)
                  {
                      if (const auto& result = _commands.find(command); result != _commands.end())
                      {
                          commandExists = true;
                          result->second.second();
                      }
                      else
                      {
                          fmt::print(fmt::fg(fmt::color::red), "{} Command not found!\n", command);
                      }
                  });
    if (!commandExists)
    {
        fmt::print(fmt::fg(fmt::color::medium_spring_green), "Try leaf help\n");
    }
    return 0;
}

void Commands::registerCommands(std::string&&                   command,
                                std::string&&                   description,
                                const std::function<int(void)>& callable)
{
    _commands[command] = {description, callable};
};

const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&
Commands::getCommands() const
{
    return _commands;
}

const std::vector<std::string>& Commands::getArgs() const
{
    return _args;
}
} // namespace Commands

namespace LeafCommands
{
LeafCommands::LeafCommands(std::vector<std::string>&& args)
    : _commands(std::make_unique<Commands::Commands>(
          Commands::Commands(std::move(args),
                             []()
                             {
                                 fmt::print("🍃 Leaf - A modern C++ project manager.\n");
                                 fmt::print("Run 'leaf help' for a list of commands.\n");
                             }))),
      _args(args)
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
    _commands->registerCommands("clean",
                                "Deletes all build files and temporary artifacts "
                                "from the project directory.",
                                [this]() { return this->clean(); });
    _commands->registerCommands("format",
                                "Automatically formats all C++ source code files "
                                "according to a consistent style.",
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
    _commands->registerCommands("publish",
                                "Formally uploads the final, release-ready "
                                "version of a package to a remote.",
                                [this]() { return this->publish(); });
    _commands->registerCommands(
        "upload",
        "Uploads a packaged library to a specified remote Conan repository.",
        [this]() { return this->upload(); });
    _commands->registerCommands(
        "doctor",
        "Checks your system to ensure all required tools (Clang, CMake,Conan) ",
        [this]() { return this->doctor(); });
    _commands->registerCommands("release",
                                "Fetches and installs in release mode all the "
                                "dependencies listed in the conanfile.py.",
                                [this]() { return this->release(); });
    _commands->registerCommands(
        "init",
        "Initializes a new Leaf project structure within an existing directory.",
        [this]() { return this->init(); });
    _commands->registerCommands("addapp",
                                "Add Executable type subproject to your project",
                                [this]() { return this->addApp(); });

    _commands->registerCommands(
        "addlib", "Add sharable library to your project", [this]() { return this->addLib(); });

    _commands->registerCommands(
        "addpkg", "Add external library to your project", [this]() { return this->addPackage(); });
    _commands->registerCommands("docs",
                                "Generate docs for your project using doxygen",
                                [this]() -> int { return this->generateDocs(); });
    _commands->registerCommands("tests",
                                "Run tests project wide using ctest",
                                [this]() -> int { return this->runTests(); });
    _commands->registerCommands("setup",
                                "Setup Clang Toolchain for C/C++ Development included "
                                "[cmake,ninja,conan,clang(mingw/msvc)]",
                                [&]() -> int { return this->setupToolChain(); });

    _commands->registerCommands(
        "update",
        "Check for leaf update and download latest version leaf from github",
        [this]() -> int { return this->startLeafUpdater(); });
};

int LeafCommands::install()
{
    Spinner spin("Installing dependencies");
    spin.start();
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
    conanInstallArgs.emplace_back("-s");
    if (std::ranges::find(_commands->getArgs(), "-r") != _commands->getArgs().end())
    {
        conanInstallArgs.emplace_back("build_type=Release");
    }
    else
    {
        conanInstallArgs.emplace_back("build_type=Debug");
    }
    if (0 != EasyProc::ProcessHandler::runExternalProcess(conanInstallArgs))
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
    };
    ;
    spin.stop();
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
        fmt::println("Project name can't be empty and can't have whitespaces in "
                     "their name!");
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

    std::map<std::string, std::string> replacements{{"%APPNAME%", project_name},
                                                    {"%LIBNAME%", lib_name},
                                                    {"startertemplate", project_name},
                                                    {"%WORKSPACE%", project_name}};
    auto                               home = sago::getConfigHome();
    if (fs::exists(home))
    {
        auto starter_template = fs::path(home) / ".leaf" / "startertemplate";
        if (!fs::exists(starter_template))
        {
            fs::create_directories(starter_template);
            Downloder::downloadGithubDirectory(
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
            auto     oldPath = templateContent.path().string();
            fs::path newPath{};
            auto     index = oldPath.find("startertemplate");
            if (index != std::string::npos)
            {
                newPath = (fs::current_path() / oldPath.substr(index));
            }
            std::string name          = newPath.filename().string();
            std::string newPathString = newPath.string();
            std::for_each(replacements.begin(),replacements.end(),
                                  [&newPathString](const auto& rep)
                                  { Utils::replaceString(newPathString, rep.first, rep.second); });
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

                std::ranges::copy(std::istreambuf_iterator<char>(in),
                                  std::istreambuf_iterator<char>{},
                                  std::back_inserter(content));
                std::for_each(replacements.begin(),replacements.end(),
                                      [&content](const auto& rep)
                                      { Utils::replaceString(content, rep.first, rep.second); });
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
    Spinner spin("Creating Package");
    spin.start();
    if (0 !=
        EasyProc::ProcessHandler::runExternalProcess({"conan", "create", ".", "-b", "missing"}))
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
    };

    return 0;
};

int LeafCommands::upload()
{
    // TODO
    return 0;
};

int LeafCommands::runTests()
{
    EasyProc::ProcessHandler::runExternalProcess(
        {"ctest", "--test-dir", ".build/debug/tests"}, false, true);
    return 0;
};

int LeafCommands::format()
{
    return 0;
};

int LeafCommands::clean()
{
    namespace fs = std::filesystem;
    Spinner spin("Running Clean build");
    spin.start();
    if (std::ranges::find(_commands->getArgs(), "-r") != _commands->getArgs().end())
    {
        if (!fs::exists(".install/Release"))
            install();
        if (0 != EasyProc::ProcessHandler::runExternalProcess(
                     {"cmake", "--preset", "release", "--fresh"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        ;
    }
    else
    {
        if (!fs::exists(".install/Debug"))
            install();
        if (0 !=
            EasyProc::ProcessHandler::runExternalProcess({"cmake", "--preset", "debug", "--fresh"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        ;
    }
    spin.stop();
    return 0;
};

int LeafCommands::release()
{
    namespace fs = std::filesystem;
    Spinner spin;
    if (fs::exists("conanfile.txt") || fs::exists("conanfile.py"))
    {
        spin.setDisplayMessage("Installing deps in release mode");
        spin.start();
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
        if (0 != EasyProc::ProcessHandler::runExternalProcess(conanInstallArgs))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        ;
    }

    spin.setDisplayMessage("Generating CMake files");
    if (fs::exists("CMakePresets.json"))
    {
        if (0 != EasyProc::ProcessHandler::runExternalProcess(
                     {"cmake", "--preset", "release", "--fresh"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        ;
    }
    else
    {
        if (0 != EasyProc::ProcessHandler::runExternalProcess({"cmake",
                                                               "-S",
                                                               ".",
                                                               "-B",
                                                               ".build/release",
                                                               "-G",
                                                               "Ninja",
                                                               "-DBUILD_TYPE=Release"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        ;
    }
    spin.setDisplayMessage("Compiling Project");
    if (0 != EasyProc::ProcessHandler::runExternalProcess({"cmake", "--build", ".build/release"}))
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
    };
    spin.stop();
    return 0;
};

int LeafCommands::addPackage()
{
    if (_args.size() <= 2)
    {
        fmt::println("error: At least one package needs to be provided!");
        return -1;
    }

    std::vector<std::pair<std::string, std::string>> packages_to_install;
    for (size_t i = 2; i < _args.size(); ++i)
    {
        const auto& arg = _args[i];
        if (arg == "-r")
        {
            continue; // Skip for now
        }

        auto        index           = arg.find('/');
        std::string package_name    = (index != std::string::npos) ? arg.substr(0, index) : arg;
        std::string package_version = (index != std::string::npos) ? arg.substr(index + 1) : "";

        if (EasyProc::ProcessHandler::runExternalProcess({"conan", "search", arg}) != 0)
        {
            fmt::println("error: Failed to search for package '{}'", arg);
            continue;
        }

        std::string              log = EasyProc::ProcessHandler::getLog();
        std::regex               pattern(package_name + R"(\/(\d+(?:\.\d+)*))");
        std::smatch              match;
        std::vector<std::string> versions;
        for (std::sregex_iterator it(log.begin(), log.end(), pattern), end; it != end; ++it)
        {
            versions.push_back(it->str());
        }

        if (versions.empty())
        {
            fmt::println("error: Could not find any versions for package '{}'", package_name);
            continue;
        }

        std::string selected_package;
        if (versions.size() > 1)
        {
            fmt::println("Found multiple versions for package '{}':", package_name);
            for (size_t j = 0; j < versions.size(); ++j)
            {
                fmt::println("{}: {}", j + 1, versions[j]);
            }
            fmt::print("Select a version to install (1-{}): ", versions.size());
            int choice;
            std::cin >> choice;
            if (choice > 0 && choice <= versions.size())
            {
                selected_package = versions[choice - 1];
            }
            else
            {
                fmt::println("error: Invalid selection.");
                continue;
            }
        }
        else
        {
            selected_package = versions[0];
        }

        index           = selected_package.find('/');
        package_name    = selected_package.substr(0, index);
        package_version = selected_package.substr(index + 1);
        packages_to_install.emplace_back(package_name, package_version);
    }

    if (packages_to_install.empty())
    {
        return 0;
    }

    std::vector<std::string> conan_lines;
    std::ifstream            in("conanfile.py");
    if (in.is_open())
    {
        std::string line;
        while (std::getline(in, line))
        {
            conan_lines.push_back(line);
        }
        in.close();
    }
    else
    {
        fmt::println("error: failed to open conanfile.py");
        return -1;
    }

    auto it = std::find(conan_lines.begin(), conan_lines.end(), "    def requirements(self):");
    if (it == conan_lines.end())
    {
        it = std::find(conan_lines.begin(), conan_lines.end(), "def requirements(self):");
    }

    if (it != conan_lines.end())
    {
        for (const auto& pkg : packages_to_install)
        {
            it = conan_lines.insert(
                it + 1, fmt::format("        self.requires(\"{}/{}\")", pkg.first, pkg.second));
        }
    }
    else
    {
        fmt::println("error: 'def requirements(self):' not found in conanfile.py");
        return -1;
    }

    std::ofstream out("conanfile.py");
    if (out.is_open())
    {
        for (const auto& line : conan_lines)
        {
            out << line << std::endl;
        }
        out.close();
    }
    else
    {
        fmt::println("error: failed to write to conanfile.py");
        return -1;
    }

    if (install() != 0)
    {
        return -1;
    }

    auto       install_log = EasyProc::ProcessHandler::getLog();
    std::regex find_package_regex(R"(find_package\(([^)]+)\))");
    std::regex target_link_regex(R"(target_link_libraries\(([^)]+)\))");

    std::vector<std::string> find_packages;
    std::vector<std::string> link_libraries;

    for (std::sregex_iterator it(install_log.begin(), install_log.end(), find_package_regex), end;
         it != end;
         ++it)
    {
        find_packages.push_back(it->str());
    }

    for (std::sregex_iterator it(install_log.begin(), install_log.end(), target_link_regex), end;
         it != end;
         ++it)
    {
        link_libraries.push_back(it->str());
    }

    auto update_cmake_lists = [&](const std::string&              path,
                                  const std::vector<std::string>& packages,
                                  const std::vector<std::string>& libs)
    {
        std::vector<std::string> lines;
        std::ifstream            cmake_in(path);
        if (cmake_in.is_open())
        {
            std::string line;
            while (std::getline(cmake_in, line))
            {
                lines.push_back(line);
            }
            cmake_in.close();

            for (const auto& pkg : packages)
            {
                lines.insert(lines.begin(), pkg);
            }

            for (auto& line : lines)
            {
                if (line.find("target_link_libraries") != std::string::npos)
                {
                    for (const auto& lib : libs)
                    {
                        line.insert(line.find_last_of(')'), " " + lib);
                    }
                }
            }

            std::ofstream cmake_out(path);
            if (cmake_out.is_open())
            {
                for (const auto& line : lines)
                {
                    cmake_out << line << std::endl;
                }
                cmake_out.close();
            }
        }
    };

    update_cmake_lists("libs/CMakeLists.txt", find_packages, {});
    update_cmake_lists("src/CMakeLists.txt", find_packages, {});

    for (const auto& entry : std::filesystem::directory_iterator("libs"))
    {
        if (entry.is_directory())
        {
            update_cmake_lists(entry.path().string() + "/CMakeLists.txt", {}, link_libraries);
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator("apps"))
    {
        if (entry.is_directory())
        {
            update_cmake_lists(entry.path().string() + "/CMakeLists.txt", {}, link_libraries);
        }
    }

    return 0;
};

int LeafCommands::removePackage()
{
    // TODO
    return 0;
};

int LeafCommands::addApp()
{
    // TODO
    // TODO
    std::string project_name{};
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your app? ");

    std::getline(std::cin, project_name);
    if (project_name.find(" ") != std::string::npos || project_name.empty())
    {
        fmt::println("App name can't be empty and can't have whitespaces in their name!");
        return 0;
    }
    namespace fs = std::filesystem;
    std::map<std::string, std::string> replacements{
        {"%APPNAME%", project_name}, {"%WORKSPACE%", fs::current_path().filename().string()}};
    auto home = sago::getConfigHome();

    if (fs::exists(home))
    {
        auto starter_template = fs::path(home) / ".leaf" / "startertemplate";
        if (!fs::exists(starter_template))
        {
            fs::create_directories(starter_template);
            Downloder::downloadGithubDirectory(
                "vishal-ahirwar", "leaf", "startertemplate", starter_template.string());
        }
        if (!fs::exists(starter_template))
        {
            fmt::println("Could not find or download start template code!");
            return 1;
        };

        for (const auto& templateContent : fs::recursive_directory_iterator(starter_template))
        {
            auto     oldPath = templateContent.path().string();
            fs::path newPath{};
            auto     index = oldPath.find("startertemplate");
            if (index != std::string::npos)
            {
                newPath = (fs::current_path() / oldPath.substr(index));
            }
            index = oldPath.find("%APPNAME%");
            if (index == std::string::npos)
            {
                continue;
            }
            newPath                   = (fs::current_path() / "apps" / oldPath.substr(index));
            std::string name          = newPath.filename().string();
            std::string newPathString = newPath.string();
            std::for_each(replacements.begin(),replacements.end(),
                                  [&newPathString](const auto& rep)
                                  { Utils::replaceString(newPathString, rep.first, rep.second); });
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

                std::ranges::copy(std::istreambuf_iterator<char>(in),
                                  std::istreambuf_iterator<char>{},
                                  std::back_inserter(content));
                std::for_each(replacements.begin(),replacements.end(),
                                      [&content](const auto& rep)
                                      { Utils::replaceString(content, rep.first, rep.second); });
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

    std::ofstream cmake("apps/CMakeLists.txt", std::ios::app);
    if (cmake.is_open())
    {
        cmake << "add_subdirectory(" << project_name << ")\n";
    }
    cmake.close();
    return 0;
};

int LeafCommands::addLib()
{
    // TODO
    std::string project_name{};
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your lib? ");

    std::getline(std::cin, project_name);
    if (project_name.find(" ") != std::string::npos || project_name.empty())
    {
        fmt::println("Lib name can't be empty and can't have whitespaces in their name!");
        return 0;
    }
    namespace fs = std::filesystem;
    std::map<std::string, std::string> replacements{
        {"%LIBNAME%", project_name}, {"%WORKSPACE%", fs::current_path().filename().string()}};
    auto home = sago::getConfigHome();

    if (fs::exists(home))
    {
        auto starter_template = fs::path(home) / ".leaf" / "startertemplate";
        if (!fs::exists(starter_template))
        {
            fs::create_directories(starter_template);
            Downloder::downloadGithubDirectory(
                "vishal-ahirwar", "leaf", "startertemplate", starter_template.string());
        }
        if (!fs::exists(starter_template))
        {
            fmt::println("Could not find or download start template code!");
            return 1;
        };

        for (const auto& templateContent : fs::recursive_directory_iterator(starter_template))
        {
            auto     oldPath = templateContent.path().string();
            fs::path newPath{};
            auto     index = oldPath.find("startertemplate");
            if (index != std::string::npos)
            {
                newPath = (fs::current_path() / oldPath.substr(index));
            }
            index = oldPath.find("%LIBNAME%");
            if (index == std::string::npos)
            {

                continue;
            }
            newPath                   = (fs::current_path() / "libs" / oldPath.substr(index));
            std::string name          = newPath.filename().string();
            std::string newPathString = newPath.string();
            std::for_each(replacements.begin(),replacements.end(),
                                  [&newPathString](const auto& rep)
                                  { Utils::replaceString(newPathString, rep.first, rep.second); });
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

                std::ranges::copy(std::istreambuf_iterator<char>(in),
                                  std::istreambuf_iterator<char>{},
                                  std::back_inserter(content));
                std::for_each(replacements.begin(),replacements.end(),
                                      [&content](const auto& rep)
                                      { Utils::replaceString(content, rep.first, rep.second); });
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

    std::ofstream cmake{"libs/CMakeLists.txt", std::ios::app};
    if (cmake.is_open())
    {
        cmake << "add_subdirectory(" << project_name << ")\n";
        cmake.close();
    }
    return 0;
};

int LeafCommands::doctor()
{
    Spinner spin("Checking required Toolchain");
    spin.start();
    std::vector<std::string> tools{"clang", "cmake", "ninja", "conan"};

    bool allToolsInstalled{false};

    std::for_each(tools.begin(),tools.end(),
                          [&allToolsInstalled](const auto& tool)
                          {
                              allToolsInstalled = EasyProc::ProcessHandler::runExternalProcess(
                                                      {tool, "--version"}) == 0;
                              if (!allToolsInstalled)
                              {
                                  fmt::println("{}", EasyProc::ProcessHandler::getLog());
                              }
                          });
    spin.stop();
    fmt::print(allToolsInstalled ? fmt::emphasis::bold | fmt::fg(fmt::color::medium_sea_green)
                                 : fmt::emphasis::underline | fmt::fg(fmt::color::crimson),
               "All tools installed : {}",
               allToolsInstalled ? "True" : "False");

    return 0;
};

int LeafCommands::build()
{

    namespace fs = std::filesystem;
    if (fs::exists("CMakePresets.json"))
    {

        if (!fs::exists(".install"))
        {
            install();
        };
        Spinner spin("Building project");
        spin.start();
        if (!fs::exists(".build/debug"))
        {
            spin.setDisplayMessage("Generating cmake files");
            if (0 != EasyProc::ProcessHandler::runExternalProcess(
                         {"cmake", "--preset", "debug", "--fresh"}))
            {
                fmt::println("{}", EasyProc::ProcessHandler::getLog());
            };
            ;
        }
        spin.setDisplayMessage("Compiling");
        if (0 != EasyProc::ProcessHandler::runExternalProcess({"cmake", "--build", ".build/debug"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        spin.stop();
    }
    else
    {

        Spinner spin("Building project");
        spin.start();
        if (!fs::exists(".install") && (fs::exists("conanfile.txt") || fs::exists("conanfile.py")))
        {
            install();
        };
        if (!fs::exists(".build/debug"))
        {
            spin.setDisplayMessage("Generating cmake files");
            if (0 != EasyProc::ProcessHandler::runExternalProcess({"cmake",
                                                                   "-S",
                                                                   ".",
                                                                   "-B",
                                                                   ".build/debug",
                                                                   "-G",
                                                                   "Ninja",
                                                                   "-DBUILD_TYPE=Debug"}))
            {
                fmt::println("{}", EasyProc::ProcessHandler::getLog());
            };
            ;
        }
        spin.setDisplayMessage("Compiling");
        if (0 != EasyProc::ProcessHandler::runExternalProcess({"cmake", "--build", ".build/debug"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        spin.stop();
    }
    return 0;
}

int LeafCommands::compile()
{
    Spinner spin("Compiling");
    spin.start();
    if (std::ranges::find(_commands->getArgs(), "-r") != _commands->getArgs().end())
    {
        spin.setDisplayMessage("Compiling in release mode");
        if (0 !=
            EasyProc::ProcessHandler::runExternalProcess({"cmake", "--build", ".build/release"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
        ;
    }
    else
    {
        spin.setDisplayMessage("Compiling in debug mode");
        if (EasyProc::ProcessHandler::runExternalProcess({"cmake", "--build", ".build/debug"}) != 0)
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        };
    }
    spin.stop();
    return 0;
}

int LeafCommands::run()
{
    build();
    namespace fs = std::filesystem;
#ifdef _WIN322
    std::string extention = ".exe";
#else
    std::string extention{};
#endif
    if (std::ranges::find(_commands->getArgs(), "-r") != _commands->getArgs().end())
    {
        auto appName = _commands->getArgs().back();
        if (appName == "-r" || _commands->getArgs().size() <= 2)
        {
            appName = fs::current_path().filename().string();
        }
        EasyProc::ProcessHandler::runExternalProcess(
            {fmt::format("./.build/release/apps/{}/{}{}", appName, appName, extention)},
            false,
            true);
    }
    else
    {
        auto appName = _commands->getArgs().back();
        if (_commands->getArgs().size() <= 2)
        {
            appName = fs::current_path().filename().string();
        }
        EasyProc::ProcessHandler::runExternalProcess(
            {fmt::format("./.build/debug/apps/{}/{}{}", appName, appName, extention)}, false, true);
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
    std::for_each(sorted_commands.begin(),sorted_commands.end(),
                          [](const auto& command)
                          {
                              fmt::print("{:<8} - ", command.first);
                              fmt::print(fmt::emphasis::faint | fmt::fg(fmt::color::floral_white),
                                         "{}\n",
                                         command.second.first);
                          });
    fmt::println("");
    return 0;
}
int LeafCommands::generateDocs()
{
    // TODO
    return -1;
}

int LeafCommands::version() const
{
    fmt::println("🍃 {} v{}", Project::PROJECT_NAME, Project::VERSION_STRING);
    return 0;
}
int LeafCommands::setupToolChain()
{
    // TODO install mingw by default on windows, clang on linux and mac and use
    // clang and mingw and generate toolchainfile to use mingw(installing libs
    // (through conan)) and clang(to build project)
    Downloder::download("mingw", "mingw");
    // TODO only on windows install msvc if user wants and genenrate toochain file
    // to use msvc(to install libs through conan) with clang(to build project)
    Downloder::download("msvc", "msvc");
    // TODO install conan binary on windows,linux,mac
    Downloder::download("conan", "conan");
    // TODO install ninja binary from github for windows,linux,mac
    Downloder::download("ninja", "ninja");
    // TODO install cmake binary from github for windows,linux,mac
    Downloder::download("cmake", "cmake");
    // TODO generate profiles for android,web and to use clang compiler on
    // windows,linux,mac
    Utils::generateProfiles();
    // TODO ask user to add leaf config into system path
    Utils::printLeafConfigPath();
    return 0;
}

int LeafCommands::exec()
{
    return _commands->exec();
}

int LeafCommands::startLeafUpdater()
{
    fmt::println("Note : LeafUpdater is not implemented yet!");

    EasyProc::ProcessHandler::runExternalProcess({"updater"}, false, true);
    
    return 0;
};

int LeafCommands::generateProfile()
{
    // Read Root CMakeLists.txt to find the compiler version, build type and then in projectRootDirectory/profiles/ generate conan profile
    // first run conan profile --detect --force then modify the profile according to compiler version and build type and save it as
    // osname_profile in projectRootDirectory/profiles/ with modified settings like
    // compiler.version,compiler.libcxx,build_type and compiler.cppstd and other settings if
    // required
    // because leaf enforces clang compiler on all platforms  but it gives user
    // choice to use other compilers too so we need to modify the profile
    // so byefault leaf will generate profile with  &:clang compiler, where & means for current project only use gcc or msvc to install dependencies through conan
    // but use clang to build the project
    // on windows leaf will download clang but user must download msvc to use leaf package manager
    // features
    //that's all:)


    std::ifstream cmake{"CMakeLists.txt"};
    if (!cmake.is_open())
    {
        Logger::Logger::log("Could not open CMakeLists.txt");
        return 1;
    };

    std::vector<std::string>cmake_lines{};
    while (cmake.good())
    {
        std::string line;
        std::getline(cmake, line);
        cmake_lines.push_back(line);
    };

    std::transform(cmake_lines.begin(),cmake_lines.end(),cmake_lines.begin(),[](std::string line)
    {
        std::transform(line.begin(),line.end(),line.begin(),[](char c){return std::tolower(c);});
        return line;
    });


    std::for_each(cmake_lines.begin(),cmake_lines.end(),[](const std::string& line)
    {
        Logger::Logger::log(line);
    });
};

} // namespace LeafCommands
