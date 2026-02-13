//
// Created by itsvi on 10/22/2025.
//

#include "commands.h"

#include <downloader.h>
#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <leafconfig.h>
#include <pystring.h>
#include <sago/platform_folders.h>
#include <spinner.h>
#include <utils.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

#include "logger.h"

namespace Leaf
{
int CommandRegistry::exec()
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

void CommandRegistry::registerCommands(std::string&&                   command,
                                       std::string&&                   description,
                                       const std::function<int(void)>& callable)
{
    _commands[command] = {description, callable};
};

const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&
CommandRegistry::getCommands() const
{
    return _commands;
}

const std::vector<std::string>& CommandRegistry::getArgs() const
{
    return _args;
}
} // namespace Leaf

namespace Leaf
{
CLI::CLI(std::vector<std::string>&& args)
    : _commands(std::make_unique<CommandRegistry>(
          CommandRegistry(std::move(args),
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

int CLI::install()
{
    Spinner spin("Installing dependencies");
    spin.start();

    // Ensure profile exists
    if (!std::filesystem::exists(Utils::getOSProfilePath()))
    {
        generateProfile();
    }

    std::vector<std::string> conanInstallArgs{"conan",
                                              "install",
                                              ".",
                                              "-of",
                                              ".install",
                                              "-b",
                                              "missing",
                                              "-pr",
                                              Utils::getOSProfilePath(),
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

int CLI::create()
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
            Downloader::downloadGithubDirectory(
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
            std::for_each(replacements.begin(),
                          replacements.end(),
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
                std::for_each(replacements.begin(),
                              replacements.end(),
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

int CLI::publish()
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

int CLI::upload()
{
    // Basic wrapper around conan upload
    // Usage: leaf upload <package_pattern> -r <remote>
    if (_args.size() < 3)
    {
        fmt::println("Usage: leaf upload <package_pattern> -r <remote>");
        return 1;
    }

    // construct args for conan upload
    // leaf upload pattern ... -> conan upload pattern ...
    std::vector<std::string> conanArgs = {"conan", "upload"};
    for (size_t i = 2; i < _args.size(); ++i)
    {
        conanArgs.push_back(_args[i]);
    }

    if (EasyProc::ProcessHandler::runExternalProcess(conanArgs) != 0)
    {
        fmt::println("Upload failed.");
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
    }

    fmt::println("Upload successful.");
    return 0;
};

int CLI::runTests()
{
    EasyProc::ProcessHandler::runExternalProcess(
        {"ctest", "--test-dir", ".build/debug/tests"}, false, true);
    return 0;
};

int CLI::format()
{
    Spinner spin("Formatting code");
    spin.start();
    // Use git ls-files or find to get all cpp/h files?
    // Or just recurse current directory.
    // simpler: use easyproc to run clang-format on known directories (apps, libs, tests, src,
    // include)

    std::vector<std::string> dirs_to_format = {"apps", "libs", "tests", "src", "include"};
    std::vector<std::string> files_to_format;

    for (const auto& dir : dirs_to_format)
    {
        if (std::filesystem::exists(dir))
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
            {
                if (entry.is_regular_file())
                {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".c" ||
                        ext == ".cc")
                    {
                        files_to_format.push_back(entry.path().string());
                    }
                }
            }
        }
    }

    if (files_to_format.empty())
    {
        spin.stop();
        fmt::println("No source files found to format.");
        return 0;
    }

    // Run clang-format in batches or one by one?
    // One by one is safer for command line length limits.
    // Or just pass -i and the file list if not too long.
    // Let's do one by one for simplicity and robustness.

    for (const auto& file : files_to_format)
    {
        EasyProc::ProcessHandler::runExternalProcess(
            {"clang-format", "-i", "-style=file", file}, false, false);
    }

    spin.stop();
    fmt::println("Formatted {} files.", files_to_format.size());
    return 0;
};

int CLI::clean()
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

int CLI::release()
{
    namespace fs = std::filesystem;
    Spinner spin;
    if (fs::exists("conanfile.txt") || fs::exists("conanfile.py"))
    {
        spin.setDisplayMessage("Installing deps in release mode");
        spin.start();

        // Ensure profile exists
        if (!std::filesystem::exists(Utils::getOSProfilePath()))
        {
            generateProfile();
        }

        std::vector<std::string> conanInstallArgs{"conan",
                                                  "install",
                                                  ".",
                                                  "-of",
                                                  ".install",
                                                  "-b",
                                                  "missing",
                                                  "-s",
                                                  "build_type=Release",
                                                  "-pr",
                                                  Utils::getOSProfilePath(),
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

int CLI::addPackage()
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

int CLI::removePackage()
{
    if (_args.size() <= 2)
    {
        fmt::println("error: At least one package needs to be provided!");
        return -1;
    }

    std::vector<std::string> packages_to_remove;
    for (size_t i = 2; i < _args.size(); ++i)
    {
        const auto& arg = _args[i];
        if (arg == "-r")
            continue;
        packages_to_remove.push_back(arg);
    }

    if (packages_to_remove.empty())
        return 0;

    // 1. Remove from conanfile.py
    std::vector<std::string> lines;
    std::ifstream            in("conanfile.py");
    if (!in.is_open())
    {
        fmt::println("error: failed to open conanfile.py");
        return -1;
    }

    std::string line;
    while (std::getline(in, line))
    {
        bool remove = false;
        for (const auto& pkg : packages_to_remove)
        {
            // Simple check: looking for pkg name in self.requires
            // e.g. self.requires("fmt/10.0.0")
            // pkg could be "fmt" or "fmt/10.0.0"
            // We'll check if the line contains the package name and "self.requires"
            if (line.find("self.requires") != std::string::npos &&
                line.find("\"" + pkg) != std::string::npos)
            {
                remove = true;
                break;
            }
        }
        if (!remove)
        {
            lines.push_back(line);
        }
    }
    in.close();

    std::ofstream out("conanfile.py");
    if (!out.is_open())
    {
        fmt::println("error: failed to write to conanfile.py");
        return -1;
    }
    for (const auto& l : lines)
    {
        out << l << "\n";
    }
    out.close();

    // 2. Run install to update dependencies
    if (install() != 0)
        return -1;

    // 3. Update CMakeLists.txt (Best effort removal)
    auto update_cmake_lists = [&](const std::string& path)
    {
        if (!std::filesystem::exists(path))
            return;
        std::vector<std::string> cmake_lines;
        std::ifstream            cmake_in(path);
        if (cmake_in.is_open())
        {
            std::string l;
            while (std::getline(cmake_in, l))
            {
                bool remove = false;
                // Remove find_package(PKG...)
                if (l.find("find_package") != std::string::npos)
                {
                    for (const auto& pkg : packages_to_remove)
                    {
                        // pkg might be "fmt/10.2.1", we just want "fmt" usually
                        std::string clean_pkg = pkg.substr(0, pkg.find('/'));
                        if (l.find(clean_pkg) != std::string::npos)
                        {
                            remove = true;
                            break;
                        }
                    }
                }
                // We won't try to remove from target_link_libraries automatically
                // because it's hard to know which target to remove from and the formatting
                // might be complex. Users might need to do this manually.
                // But `addPackage` adds it to target_link_libraries...
                // Let's at least warn or try simple removal if on same line.

                if (!remove)
                {
                    cmake_lines.push_back(l);
                }
            }
            cmake_in.close();

            std::ofstream cmake_out(path);
            for (const auto& l : cmake_lines)
                cmake_out << l << "\n";
        }
    };

    update_cmake_lists("libs/CMakeLists.txt");
    update_cmake_lists("src/CMakeLists.txt");
    // Also recurse into apps/ and libs/ subdirs if we want to be thorough
    // but for now this is a basic implementation.

    fmt::println("Packages removed from conanfile.py. Please check CMakeLists.txt for any leftover "
                 "linking.");
    return 0;
};

int CLI::addApp()
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
            Downloader::downloadGithubDirectory(
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
            std::for_each(replacements.begin(),
                          replacements.end(),
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
                std::for_each(replacements.begin(),
                              replacements.end(),
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

int CLI::addLib()
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
            Downloader::downloadGithubDirectory(
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
            std::for_each(replacements.begin(),
                          replacements.end(),
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
                std::for_each(replacements.begin(),
                              replacements.end(),
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

int CLI::doctor()
{
    Spinner spin("Checking required Toolchain");
    spin.start();
    std::vector<std::string> tools{"clang", "cmake", "ninja", "conan"};

    bool allToolsInstalled{false};

    std::for_each(tools.begin(),
                  tools.end(),
                  [&allToolsInstalled](const auto& tool)
                  {
                      allToolsInstalled =
                          EasyProc::ProcessHandler::runExternalProcess({tool, "--version"}) == 0;
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

int CLI::build()
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
            spin.stop(); // Stop spinner to run install (which has its own spinner)
            install();
            spin.start(); // Restart spinner

            // Check if presets were generated
            if (fs::exists("CMakePresets.json"))
            {
                spin.setDisplayMessage("Generating cmake files (using presets)");
                if (0 != EasyProc::ProcessHandler::runExternalProcess(
                             {"cmake", "--preset", "debug", "--fresh"}))
                {
                    fmt::println("{}", EasyProc::ProcessHandler::getLog());
                }
                spin.setDisplayMessage("Compiling");
                if (0 != EasyProc::ProcessHandler::runExternalProcess(
                             {"cmake", "--build", ".build/debug"}))
                {
                    fmt::println("{}", EasyProc::ProcessHandler::getLog());
                }
                spin.stop();
                return 0;
            }
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

int CLI::compile()
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

int CLI::run()
{
    build();
    namespace fs = std::filesystem;
#ifdef _WIN32
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

int CLI::init()
{
    namespace fs             = std::filesystem;
    std::string project_name = fs::current_path().filename().string();
    std::string lib_name     = project_name + "lib";

    // Allow overriding name via args? For now, simpler is better.

    if (_args.size() > 2)
    {
        project_name = _args[2];
        lib_name     = project_name + "lib";
    }

    fmt::println("Initializing project '{}' in current directory.", project_name);

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
            Downloader::downloadGithubDirectory(
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
                // Determine new path relative to current directory
                // startertemplate/foo/bar -> ./foo/bar
                // We need to skip "startertemplate/"
                std::string relPath =
                    oldPath.substr(index + std::string("startertemplate").length());
                // Remove leading separator if any
                if (!relPath.empty() && (relPath[0] == '/' || relPath[0] == '\\'))
                {
                    relPath = relPath.substr(1);
                }

                if (relPath.empty())
                    continue; // Skip root folder itself

                newPath = fs::current_path() / relPath;
            }

            std::string name          = newPath.filename().string();
            std::string newPathString = newPath.string();
            std::for_each(replacements.begin(),
                          replacements.end(),
                          [&newPathString](const auto& rep)
                          { Utils::replaceString(newPathString, rep.first, rep.second); });

            if (fs::exists(newPathString))
            {
                fmt::println("Skipping existing file/directory: {}", newPathString);
                continue;
            }

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
                std::for_each(replacements.begin(),
                              replacements.end(),
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
int CLI::generateDocs()
{
    if (!std::filesystem::exists("Doxyfile"))
    {
        fmt::println("No Doxyfile found. Generating a default one...");
        if (EasyProc::ProcessHandler::runExternalProcess({"doxygen", "-g"}) != 0)
        {
            fmt::println("Failed to generate Doxyfile. Is doxygen installed?");
            return 1;
        }
        fmt::println("Doxyfile generated. Please customize it and run 'leaf docs' again.");
        return 0;
    }

    Spinner spin("Generating documentation");
    spin.start();
    if (EasyProc::ProcessHandler::runExternalProcess({"doxygen", "Doxyfile"}) != 0)
    {
        spin.stop();
        fmt::println("Failed to generate documentation.");
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
    }
    spin.stop();
    fmt::println(
        "Documentation generated in 'html/latex' directory (check Doxyfile for output path).");
    return 0;
}

int CLI::version() const
{
    fmt::println("🍃 {} v{}", Project::PROJECT_NAME, Project::VERSION_STRING);
    return 0;
}
int CLI::setupToolChain()
{
    // TODO install mingw by default on windows, clang on linux and mac and use
    // clang and mingw and generate toolchainfile to use mingw(installing libs
    // (through conan)) and clang(to build project)
    Downloader::download("mingw", "mingw");
    // TODO only on windows install msvc if user wants and genenrate toochain file
    // to use msvc(to install libs through conan) with clang(to build project)
    Downloader::download("msvc", "msvc");
    // TODO install conan binary on windows,linux,mac
    Downloader::download("conan", "conan");
    // TODO install ninja binary from github for windows,linux,mac
    Downloader::download("ninja", "ninja");
    // TODO install cmake binary from github for windows,linux,mac
    Downloader::download("cmake", "cmake");
    // TODO generate profiles for android,web and to use clang compiler on
    // windows,linux,mac
    this->generateProfile();
    // TODO ask user to add leaf config into system path
    Utils::printLeafConfigPath();
    return 0;
}

int CLI::exec()
{
    return _commands->exec();
}

int CLI::startLeafUpdater()
{
    fmt::println("Note : LeafUpdater is not implemented yet!");

    EasyProc::ProcessHandler::runExternalProcess({"updater"}, false, true);

    return 0;
};

int CLI::generateProfile()
{

    std::string cpp_std    = "20";
    std::string compiler   = "clang";
    std::string build_type = "Release";

    std::ifstream cmake{"CMakeLists.txt"};
    if (cmake.is_open())
    {
        std::string line;
        std::regex  std_regex(R"(set\s*\(\s*CMAKE_CXX_STANDARD\s+["']?(\d+)["']?\s*\))",
                             std::regex_constants::icase);
        std::regex  comp_regex(R"(set\s*\(\s*CMAKE_CXX_COMPILER\s+["']?([^\s\)]+)["']?\s*\))",
                              std::regex_constants::icase);
        std::regex  build_regex(R"(set\s*\(\s*CMAKE_BUILD_TYPE\s+["']?([^\s\)]+)["']?\s*\))",
                               std::regex_constants::icase);

        while (std::getline(cmake, line))
        {
            std::smatch match;
            if (std::regex_search(line, match, std_regex))
            {
                cpp_std = match[1].str();
            }
            if (std::regex_search(line, match, comp_regex))
            {
                compiler = match[1].str();
            }
            if (std::regex_search(line, match, build_regex))
            {
                build_type = match[1].str();
            }
        }
        cmake.close();
    }

    if (EasyProc::ProcessHandler::runExternalProcess(
            {"conan", "profile", "detect", "--force"}, true, false) != 0)
    {
        Leaf::Logger::log("Conan profile detect failed. Ensure Conan is installed.");
        return 1;
    }

    if (EasyProc::ProcessHandler::runExternalProcess(
            {"conan", "profile", "path", "default"}, true, false) != 0)
    {
        Leaf::Logger::log("Failed to get conan profile path.");
        return 1;
    }

    std::string profile_path = EasyProc::ProcessHandler::getLog();
    profile_path.erase(std::remove(profile_path.begin(), profile_path.end(), '\n'),
                       profile_path.end());
    profile_path.erase(std::remove(profile_path.begin(), profile_path.end(), '\r'),
                       profile_path.end());

    std::ifstream default_profile(profile_path);
    if (!default_profile.is_open())
    {
        Leaf::Logger::log("Could not open default conan profile at: " + profile_path);
        return 1;
    }

    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(default_profile, line))
    {
        if (line.find("compiler.cppstd=") != std::string::npos)
        {
            lines.push_back("compiler.cppstd=" + cpp_std);
        }
        else if (line.find("build_type=") != std::string::npos)
        {
            lines.push_back("build_type=" + build_type);
        }
        else
        {
            lines.push_back(line);
        }
    }
    lines.push_back("&:compiler=clang");
    if (EasyProc::ProcessHandler::runExternalProcess({"clang", "-v"}) != 0)
    {
        Logger::log("Failed to get clang compiler info.");
        return 1;
    };
    std::string              log{EasyProc::ProcessHandler::getLog()};
    std::vector<std::string> clang_logs_lines{};
    pystring::splitlines(log, clang_logs_lines);

    std::string first_line = clang_logs_lines.front();

    std::regex  version_regex(R"(clang version ([0-9]+(\.[0-9]+)*))");
    std::smatch match;

    if (!std::regex_search(first_line, match, version_regex))
    {
        Logger::log("Failed to parse clang version.");
        return 1;
    }

    std::string clang_compiler_version = match[1];
    lines.push_back(fmt::format("&:compiler.version={}", pystring::split(clang_compiler_version,".").front()));
    default_profile.close();

    std::string os_profile;
#ifdef _WIN32
    os_profile = "profiles/windows_profile";
#elif defined(__APPLE__)
    os_profile = "profiles/macos_profile";
    lines.push_back("[conf]");
    lines.push_back("tools.system.package_manager:mode=install");
    lines.push_back("tools.system.package_manager:sudo=True");
#else
    os_profile = "profiles/linux_profile";
    lines.push_back("[conf]");
    lines.push_back("tools.system.package_manager:mode=install");
    lines.push_back("tools.system.package_manager:sudo=True");
#endif

    std::filesystem::create_directories("profiles");
    std::ofstream out(os_profile);
    if (!out.is_open())
    {
        Leaf::Logger::log("Could not open output profile file for writing: " + os_profile);
        return 1;
    }

    for (const auto& l : lines)
    {
        out << l << "\n";
    }
    out.close();

    return 0;
}

} // namespace Leaf
