//
// Created by itsvi on 10/22/2025.
//

#include "commands.h"

#include <downloader.h>
#include <easyproc.h>
#include <fmt/base.h>
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
#include <optional>
#include <regex>

#include "logger.h"

namespace Leaf
{
namespace
{
bool isReleaseMode(const commandregistry& registry)
{
    return registry.hasOption("release");
}

std::optional<std::string> getAppOption(const commandregistry& registry)
{
    return registry.getOptionValue("app");
}

std::optional<std::string> getTargetOption(const commandregistry& registry)
{
    return registry.getOptionValue("target");
}

std::string detectDefaultAppName()
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
} // namespace

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
                                "Deprecated alias for `leaf build --release`.",
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
    if (isReleaseMode(*_commands))
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
    if (isReleaseMode(*_commands))
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
    fmt::println("`leaf release` is deprecated. Use `leaf build --release`.");
    namespace fs = std::filesystem;
    if (fs::exists("conanfile.txt") || fs::exists("conanfile.py"))
    {
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
        if (EasyProc::ProcessHandler::runExternalProcess(conanInstallArgs) != 0)
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
            return 1;
        }
    }

    if (fs::exists("CMakePresets.json"))
    {
        if (EasyProc::ProcessHandler::runExternalProcess({"cmake", "--preset", "release"}) != 0)
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
            return 1;
        }
    }
    else if (EasyProc::ProcessHandler::runExternalProcess({"cmake",
                                                           "-S",
                                                           ".",
                                                           "-B",
                                                           ".build/release",
                                                           "-G",
                                                           "Ninja",
                                                           "-DCMAKE_BUILD_TYPE=Release"}) != 0)
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
    }

    if (EasyProc::ProcessHandler::runExternalProcess({"cmake", "--build", ".build/release"}) != 0)
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
    }
    return 0;
};

int CLI::addPackage()
{
    const auto& package_args = _commands->getPositionals();
    if (package_args.empty())
    {
        fmt::println("Usage: leaf addpkg <pkg1> [pkg2] ... [--release]");
        return 1;
    }

    std::vector<std::string> packages_to_install{};
    for (const auto& package : package_args)
    {
        const std::string search_query =
            package.find('/') == std::string::npos ? package + "/*" : package;
        if (EasyProc::ProcessHandler::runExternalProcess({"conan", "search", search_query}) != 0)
        {
            fmt::println("error: package '{}' was not found in configured conan remotes.", package);
            continue;
        }
        const std::string normalized =
            package.find('/') == std::string::npos ? package + "/[>=0]" : package;
        packages_to_install.push_back(normalized);
    }

    if (packages_to_install.empty())
    {
        return 1;
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
            it = conan_lines.insert(it + 1, fmt::format("        self.requires(\"{}\")", pkg));
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

    namespace fs                   = std::filesystem;
    const bool        release_mode = isReleaseMode(*_commands);
    const std::string mode         = release_mode ? "release" : "debug";
    const std::string build_dir    = fmt::format(".build/{}", mode);
    std::string       app_target{};
    if (const auto target = getTargetOption(*_commands); target.has_value())
    {
        app_target = *target;
    }
    else if (const auto app = getAppOption(*_commands); app.has_value())
    {
        app_target = *app;
    }
    else if (!_commands->getPositionals().empty())
    {
        app_target = _commands->getPositionals().front();
    }
    auto path = mode == "release" ? "build/Release" : "build/Debug";
    if (!fs::exists(fmt::format(".install/{}", path)) &&
        (fs::exists("conanfile.txt") || fs::exists("conanfile.py")))
    {
        if (install() != 0)
        {
            return 1;
        }
    }

    Spinner spin("Building project");
    spin.start();
    if (fs::exists("CMakePresets.json"))
    {
        if (!fs::exists(build_dir))
        {
            spin.setDisplayMessage("Generating cmake files");
            if (EasyProc::ProcessHandler::runExternalProcess(
                    {"cmake", "--preset", mode, "--fresh"}) != 0)
            {
                spin.stop();
                fmt::println("{}", EasyProc::ProcessHandler::getLog());
                return 1;
            }
        }
    }
    else
    {
        if (!fs::exists(build_dir))
        {
            spin.setDisplayMessage("Generating cmake files");
            if (EasyProc::ProcessHandler::runExternalProcess(
                    {"cmake",
                     "-S",
                     ".",
                     "-B",
                     build_dir,
                     "-G",
                     "Ninja",
                     fmt::format("-DCMAKE_BUILD_TYPE={}", release_mode ? "Release" : "Debug")}) !=
                0)
            {
                spin.stop();
                fmt::println("{}", EasyProc::ProcessHandler::getLog());
                return 1;
            }
        }
    }

    spin.setDisplayMessage("Compiling");
    std::vector<std::string> build_args{"cmake", "--build", build_dir};
    if (!app_target.empty())
    {
        build_args.push_back("--target");
        build_args.push_back(app_target);
    }
    if (EasyProc::ProcessHandler::runExternalProcess(build_args) != 0)
    {
        spin.stop();
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
    }
    spin.stop();
    return 0;
}

int CLI::compile()
{
    return build();
}

int CLI::run()
{
    if (build() != 0)
    {
        return 1;
    }
    namespace fs = std::filesystem;
#ifdef _WIN32
    std::string extention = ".exe";
#else
    std::string extention{};
#endif
    std::string appName{};
    if (const auto target = getTargetOption(*_commands); target.has_value())
    {
        appName = *target;
    }
    else if (const auto app = getAppOption(*_commands); app.has_value())
    {
        appName = *app;
    }
    else if (!_commands->getPositionals().empty())
    {
        appName = _commands->getPositionals().front();
    }
    else
    {
        appName = detectDefaultAppName();
    }

    const std::string mode = isReleaseMode(*_commands) ? "release" : "debug";
    const auto        app_path =
        fs::path(".build") / mode / "apps" / appName / fmt::format("{}{}", appName, extention);
    if (!fs::exists(app_path))
    {
        fmt::println("App binary not found: {}", app_path.string());
        fmt::println("Use `leaf run --app <name>` or `leaf run --target <name>`.");
        return 1;
    }

    if (EasyProc::ProcessHandler::runExternalProcess({app_path.string()}, false, true) != 0)
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
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
    auto run = [](const std::vector<std::string>& cmd, bool show_log = true) -> int
    { return EasyProc::ProcessHandler::runExternalProcess(cmd, false, show_log); };

    auto exists = [](const std::vector<std::string>& probe) -> bool
    { return EasyProc::ProcessHandler::runExternalProcess(probe, false, false) == 0; };

    auto tryCommands = [&](const std::vector<std::vector<std::string>>& commands) -> bool
    {
        for (const auto& cmd : commands)
        {
            if (run(cmd) == 0)
            {
                return true;
            }
        }
        return false;
    };

    bool setup_ok = true;

#ifdef _WIN32
    auto hasArg = [&](const std::string& flag) -> bool
    { return std::ranges::find(_commands->getArgs(), flag) != _commands->getArgs().end(); };

    const bool use_mingw = hasArg("--mingw") || hasArg("mingw");

    if (use_mingw)
    {
        if (!exists({"g++", "--version"}))
        {
            fmt::println("Installing MinGW toolchain...");
            const bool mingw_ok = tryCommands(
                {{"winget", "install", "-e","--id", "BrechtSanders.WinLibs.POSIX.UCRT"},
                 {"choco", "install", "mingw", "-y"},
                 {"scoop", "install", "mingw"}});
            setup_ok &= mingw_ok;
            if (!mingw_ok)
            {
                Leaf::Logger::log("Failed to install MinGW toolchain.");
            }
        }
    }
    else
    {
        if (!exists({"cmd", "/c", "where", "cl"}))
        {
            fmt::println("Installing MSVC build tools...");

            //Download this file and use this to install build tools silently with all workloads and components needed for C++ development https://github.com/vishal-ahirwar/flick/blob/master/res/flick.vsconfig --config flick.vsconfig
            fmt::println("Installing MSVC build tools...");
            Leaf::Logger::log("You may need to select 'Desktop development with C++' workload component in installer.");
            Leaf::Logger::log("If this fails, try installing manually from https://visualstudio.microsoft.com/downloads/ (select 'Build Tools' workload) or use --mingw option to install mingw toolchain instead.");
            const bool msvc_ok = tryCommands({
                {"winget", "install", "--id", "Microsoft.VisualStudio.2022.BuildTools", "-e"},
                {"choco", "install", "visualstudio2022buildtools", "-y"}
            });
            setup_ok &= msvc_ok;
            if (!msvc_ok)
            {
                Leaf::Logger::log("Failed to install MSVC build tools.");
            }

        }

        if (!exists({"clang", "--version"}))
        {
            fmt::println("Installing clang...");
            const bool clang_ok = tryCommands({{"winget", "install", "--id", "LLVM.LLVM", "-e"},
                                               {"choco", "install", "llvm", "-y"},
                                               {"scoop", "install", "llvm"}});
            setup_ok &= clang_ok;
            if (!clang_ok)
            {
                Leaf::Logger::log("Failed to install clang.");
            }
        }
    }

    if (!exists({"cmake", "--version"}))
    {
        fmt::println("Installing cmake...");
        const bool cmake_ok = tryCommands({{"winget", "install", "--id", "Kitware.CMake", "-e"},
                                           {"choco", "install", "cmake", "-y"},
                                           {"scoop", "install", "cmake"}});
        setup_ok &= cmake_ok;
        if (!cmake_ok)
        {
            Leaf::Logger::log("Failed to install cmake.");
        }
    }

    if (!exists({"ninja", "--version"}))
    {
        fmt::println("Installing ninja...");
        const bool ninja_ok = tryCommands({{"winget", "install", "--id", "Ninja-build.Ninja", "-e"},
                                           {"choco", "install", "ninja", "-y"},
                                           {"scoop", "install", "ninja"}});
        setup_ok &= ninja_ok;
        if (!ninja_ok)
        {
            Leaf::Logger::log("Failed to install ninja.");
        }
    }

    if (!exists({"conan", "--version"}))
    {
        fmt::println("Installing conan...");
        const bool conan_ok = tryCommands({{"winget",
                                            "install",
                                            "-e",
                                            "--id",
                                            "JFrog.Conan"},
                                           {"python", "-m", "pip", "install", "--user", "conan"},
                                           {"pip", "install", "--user", "conan"}});
        setup_ok &= conan_ok;
        if (!conan_ok)
        {
            Leaf::Logger::log("Failed to install conan.");
        }
    }
#else
    const bool has_brew   = exists({"brew", "--version"});
    const bool has_apt    = exists({"apt-get", "--version"});
    const bool has_dnf    = exists({"dnf", "--version"});
    const bool has_pacman = exists({"pacman", "--version"});
    const bool has_zypper = exists({"zypper", "--version"});
    const bool has_apk    = exists({"apk", "--version"});

    auto installWithManager = [&](const std::vector<std::string>& packages) -> bool
    {
        if (packages.empty())
        {
            return true;
        }
        if (has_brew)
        {
            std::vector<std::string> cmd{"brew", "install"};
            cmd.insert(cmd.end(), packages.begin(), packages.end());
            return run(cmd) == 0;
        }
        if (has_apt)
        {
            run({"sudo", "apt-get", "update"});
            std::vector<std::string> cmd{"sudo", "apt-get", "install", "-y"};
            cmd.insert(cmd.end(), packages.begin(), packages.end());
            return run(cmd) == 0 || [&]()
            {
                std::vector<std::string> root_cmd{"apt-get", "install", "-y"};
                root_cmd.insert(root_cmd.end(), packages.begin(), packages.end());
                return run(root_cmd) == 0;
            }();
        }
        if (has_dnf)
        {
            std::vector<std::string> cmd{"sudo", "dnf", "install", "-y"};
            cmd.insert(cmd.end(), packages.begin(), packages.end());
            return run(cmd) == 0 || [&]()
            {
                std::vector<std::string> root_cmd{"dnf", "install", "-y"};
                root_cmd.insert(root_cmd.end(), packages.begin(), packages.end());
                return run(root_cmd) == 0;
            }();
        }
        if (has_pacman)
        {
            std::vector<std::string> cmd{"sudo", "pacman", "-S", "--noconfirm"};
            cmd.insert(cmd.end(), packages.begin(), packages.end());
            return run(cmd) == 0 || [&]()
            {
                std::vector<std::string> root_cmd{"pacman", "-S", "--noconfirm"};
                root_cmd.insert(root_cmd.end(), packages.begin(), packages.end());
                return run(root_cmd) == 0;
            }();
        }
        if (has_zypper)
        {
            std::vector<std::string> cmd{"sudo", "zypper", "--non-interactive", "install"};
            cmd.insert(cmd.end(), packages.begin(), packages.end());
            return run(cmd) == 0 || [&]()
            {
                std::vector<std::string> root_cmd{"zypper", "--non-interactive", "install"};
                root_cmd.insert(root_cmd.end(), packages.begin(), packages.end());
                return run(root_cmd) == 0;
            }();
        }
        if (has_apk)
        {
            std::vector<std::string> cmd{"sudo", "apk", "add"};
            cmd.insert(cmd.end(), packages.begin(), packages.end());
            return run(cmd) == 0 || [&]()
            {
                std::vector<std::string> root_cmd{"apk", "add"};
                root_cmd.insert(root_cmd.end(), packages.begin(), packages.end());
                return run(root_cmd) == 0;
            }();
        }
        return false;
    };

#if defined(__APPLE__)
    if (!exists({"clang", "--version"}) && !exists({"xcrun", "clang", "--version"}))
    {
        fmt::println("Installing clang toolchain...");
        const bool clang_ok = (has_brew && run({"brew", "install", "llvm"}) == 0) ||
                              run({"xcode-select", "--install"}) == 0;
        setup_ok &= clang_ok;
        if (!clang_ok)
        {
            Leaf::Logger::log("Failed to install clang toolchain.");
        }
    }
#else
    if (!exists({"clang", "--version"}))
    {
        fmt::println("Installing clang...");
        const bool clang_ok = installWithManager({"clang"});
        setup_ok &= clang_ok;
        if (!clang_ok)
        {
            Leaf::Logger::log("Failed to install clang.");
        }
    }
#endif

    if (!exists({"cmake", "--version"}))
    {
        fmt::println("Installing cmake...");
        const bool cmake_ok = installWithManager({"cmake"});
        setup_ok &= cmake_ok;
        if (!cmake_ok)
        {
            Leaf::Logger::log("Failed to install cmake.");
        }
    }

    if (!exists({"ninja", "--version"}))
    {
        fmt::println("Installing ninja...");
        bool ninja_ok = false;
        if (has_apt)
        {
            ninja_ok = installWithManager({"ninja-build"});
        }
        else if (has_dnf)
        {
            ninja_ok = installWithManager({"ninja-build"});
        }
        else
        {
            ninja_ok = installWithManager({"ninja"});
        }
        setup_ok &= ninja_ok;
        if (!ninja_ok)
        {
            Leaf::Logger::log("Failed to install ninja.");
        }
    }

    if (!exists({"conan", "--version"}))
    {
        fmt::println("Installing conan...");
        bool conan_ok = false;
        if (has_brew)
        {
            conan_ok = run({"brew", "install", "conan"}) == 0;
        }
        conan_ok = conan_ok || tryCommands({{"pip3", "install", "--user", "conan"},
                                            {"python3", "-m", "pip", "install", "--user", "conan"},
                                            {"pip", "install", "--user", "conan"}});
        setup_ok &= conan_ok;
        if (!conan_ok)
        {
            Leaf::Logger::log("Failed to install conan.");
        }
    }
#endif

    this->generateProfile();
    if (!setup_ok)
    {
        Leaf::Logger::log("Toolchain setup completed with one or more install failures.");
    }
    Utils::printLeafConfigPath();
    return setup_ok ? 0 : 1;
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
    lines.push_back(
        fmt::format("&:compiler.version={}", pystring::split(clang_compiler_version, ".").front()));
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
