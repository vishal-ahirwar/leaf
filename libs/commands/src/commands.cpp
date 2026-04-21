//
// Core CLI: constructor, exec, help, version, and shared helpers
//

#include "commands.h"

#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <leafconfig.h>
#include <utils.h>

#include <algorithm>
#include <filesystem>
#include <map>
namespace Leaf
{

bool CLI::isReleaseMode() const
{
    return _commands->hasOption("release");
}

bool CLI::isVerboseMode() const
{
    return _commands->hasOption("verbose");
}

std::optional<std::string> CLI::getAppOption() const
{
    return _commands->getOptionValue("app");
}

std::optional<std::string> CLI::getTargetOption() const
{
    return _commands->getOptionValue("target");
}

std::string CLI::detectDefaultAppName() const
{
    namespace fs = std::filesystem;
    std::vector<std::string> apps{};
    if (fs::exists("apps"))
    {
        for (const auto& entry : fs::directory_iterator("apps"))
        {
            if (entry.is_directory())
            {
                apps.push_back(entry.path().filename().string());
            }
        }
    }
    if (apps.size() == 1)
    {
        return apps.front();
    }
    return fs::current_path().filename().string();
}

CLI::CLI(std::vector<std::string>&& args)
    : _commands(std::make_unique<commandregistry>(
          commandregistry(args,
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
        "version", "Display leaf current version.", [this]() { return this->version(); });
    _commands->registerCommands("clean",
                                "Deletes all build artifacts and reconfigures the project.",
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
                                "Compiles the entire project and its dependencies.",
                                [this]() { return this->build(); });
    _commands->registerCommands("compile",
                                "Builds all targets in the project without running them.",
                                [this]() { return this->compile(); });
    _commands->registerCommands(
        "run",
        "Compiles and executes the main application target of your project.",
        [this]() { return this->run(); });
    _commands->registerCommands("publish",
                                "Creates a release-ready Conan package from your project.",
                                [this]() { return this->publish(); });
    _commands->registerCommands(
        "upload",
        "Uploads a packaged library to a specified remote Conan repository.",
        [this]() { return this->upload(); });
    _commands->registerCommands(
        "doctor",
        "Checks your system to ensure all required tools (Clang, CMake, Conan) are installed.",
        [this]() { return this->doctor(); });
    _commands->registerCommands(
        "init",
        "Initializes a new Leaf project structure within an existing directory.",
        [this]() { return this->init(); });
    _commands->registerCommands("addapp",
                                "Add an executable sub-project to your project.",
                                [this]() { return this->addApp(); });

    _commands->registerCommands(
        "addlib", "Add a shared library to your project.", [this]() { return this->addLib(); });

    _commands->registerCommands(
        "addpkg", "Add external library to your project.", [this]() { return this->addPackage(); });

    _commands->registerCommands("remove",
                                "Remove a package dependency from your project.",
                                [this]() { return this->removePackage(); });

    _commands->registerCommands("docs",
                                "Generate docs for your project using doxygen.",
                                [this]() -> int { return this->generateDocs(); });
    _commands->registerCommands("tests",
                                "Run tests project-wide using ctest.",
                                [this]() -> int { return this->runTests(); });
    _commands->registerCommands("setup",
                                "Setup Clang Toolchain for C/C++ Development including "
                                "[cmake, ninja, conan, clang (mingw/msvc)].",
                                [this]() -> int { return this->setupToolChain(); });

    _commands->registerCommands(
        "update",
        "Check for leaf updates and download the latest version from GitHub.",
        [this]() -> int { return this->startLeafUpdater(); });

    _commands->registerCommands("search",
                                "Search for packages in the Conan package registry.",
                                [this]() -> int { return this->search(); });

    _commands->registerCommands("info",
                                "Display detailed information about a specific package.",
                                [this]() -> int { return this->info(); });

    _commands->registerCommands("tree",
                                "Display the dependency tree of the current project.",
                                [this]() -> int { return this->depTree(); });

    _commands->registerCommands(
        "server",
        "Manage a lightweight Leaf package server for hosting Conan packages.",
        [this]() -> int { return this->server(); });
};

int CLI::exec()
{
    return _commands->exec();
}

int CLI::help()
{
    fmt::print("\n🍃 Leaf - v{} by {}\n", Project::VERSION_STRING, Project::COMPANY_NAME);
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "A modern, fast, and intuitive project/package manager for C++\n\n");
    std::map<std::string, std::pair<std::string, std::function<int()>>> sorted_commands{
        _commands->getCommands().begin(), _commands->getCommands().end()};
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "Available Commands\n");
    std::for_each(sorted_commands.begin(),
                  sorted_commands.end(),
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

int CLI::version() const
{
    fmt::println("🍃 {} v{}", Project::PROJECT_NAME, Project::VERSION_STRING);
    return 0;
}

} // namespace Leaf
