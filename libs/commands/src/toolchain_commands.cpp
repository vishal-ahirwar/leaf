//
// Toolchain management: doctor, format, setup, profile generation,
// tests, docs, updater
//

#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <progress.h>
#include <pystring.h>
#include <utils.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <regex>
#include <string>
#include <vector>

#include "commands.h"
#include "logger.h"

namespace Leaf
{

int CLI::doctor()
{
    progress::Spinner spin("Checking required Toolchain");
    if (!isVerboseMode()) spin.start();

    std::vector<std::string> tools{"clang", "cmake", "ninja", "conan","ccache"};
    bool                     allToolsInstalled = true;

    for (const auto& tool : tools)
    {
        bool installed = EasyProc::ProcessHandler::runExternalProcess({tool, "--version"}, !isVerboseMode(), isVerboseMode()) == 0;
        if (!installed)
        {
            Leaf::Logger::error(fmt::format("'{}' is not installed or not in PATH.", tool));
        }
        else
        {
            Leaf::Logger::success(fmt::format("'{}' found.", tool));
        }
        allToolsInstalled = allToolsInstalled && installed;
    }

    if (!isVerboseMode()) spin.stop();
    fmt::print(allToolsInstalled ? fmt::emphasis::bold | fmt::fg(fmt::color::medium_sea_green)
                                 : fmt::emphasis::underline | fmt::fg(fmt::color::crimson),
               "\nAll tools installed: {}\n",
               allToolsInstalled ? "True" : "False");
    return allToolsInstalled ? 0 : 1;
}

int CLI::format()
{
    progress::Spinner spin("Formatting code");
    if (!isVerboseMode()) spin.start();

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
        if (!isVerboseMode()) spin.stop();
        fmt::println("No source files found to format.");
        return 0;
    }

    for (const auto& file : files_to_format)
    {
        EasyProc::ProcessHandler::runExternalProcess(
            {"clang-format", "-i", "-style=file", file}, false, false);
    }

    if (!isVerboseMode()) spin.stop();
    Leaf::Logger::success(fmt::format("Formatted {} files.", files_to_format.size()));
    return 0;
}

int CLI::runTests()
{
    EasyProc::ProcessHandler::runExternalProcess(
        {"ctest", "--test-dir", ".build/debug/tests"}, !isVerboseMode(), true); // tests always show log
    return 0;
}

int CLI::generateDocs()
{
    if (!std::filesystem::exists("Doxyfile"))
    {
        fmt::println("No Doxyfile found. Generating a default one...");
        if (EasyProc::ProcessHandler::runExternalProcess({"doxygen", "-g"}) != 0)
        {
            Leaf::Logger::error("Failed to generate Doxyfile. Is doxygen installed?");
            return 1;
        }
        Leaf::Logger::info("Doxyfile generated. Please customize it and run 'leaf docs' again.");
        return 0;
    }

    progress::Spinner spin("Generating documentation");
    if (!isVerboseMode()) spin.start();
    if (EasyProc::ProcessHandler::runExternalProcess({"doxygen", "Doxyfile"}, !isVerboseMode(), isVerboseMode()) != 0)
    {
        if (!isVerboseMode()) spin.stop();
        Leaf::Logger::error("Failed to generate documentation.");
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
    }
    if (!isVerboseMode()) spin.stop();
    Leaf::Logger::success("Documentation generated (check Doxyfile for output path).");
    return 0;
}

int CLI::startLeafUpdater()
{
    Leaf::Logger::info("Checking for leaf updates...");
    EasyProc::ProcessHandler::runExternalProcess({"updater"}, false, true);
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
                return true;
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
                {{"winget", "install", "-e", "--id", "BrechtSanders.WinLibs.POSIX.UCRT"},
                 {"choco", "install", "mingw", "-y"},
                 {"scoop", "install", "mingw"}});
            setup_ok &= mingw_ok;
            if (!mingw_ok)
                Leaf::Logger::error("Failed to install MinGW toolchain.");
        }
    }
    else
    {
        if (!exists({"cmd", "/c", "where", "cl"}))
        {
            fmt::println("Installing MSVC build tools...");
            Leaf::Logger::info(
                "You may need to select 'Desktop development with C++' workload component.");
            Leaf::Logger::info(
                "If this fails, try installing manually from "
                "https://visualstudio.microsoft.com/downloads/ or use --mingw option.");
            const bool msvc_ok = tryCommands(
                {{"winget", "install", "--id", "Microsoft.VisualStudio.2022.BuildTools", "-e"},
                 {"choco", "install", "visualstudio2022buildtools", "-y"}});
            setup_ok &= msvc_ok;
            if (!msvc_ok)
                Leaf::Logger::error("Failed to install MSVC build tools.");
        }

        if (!exists({"clang", "--version"}))
        {
            fmt::println("Installing clang...");
            const bool clang_ok = tryCommands({{"winget", "install", "--id", "LLVM.LLVM", "-e"},
                                               {"choco", "install", "llvm", "-y"},
                                               {"scoop", "install", "llvm"}});
            setup_ok &= clang_ok;
            if (!clang_ok)
                Leaf::Logger::error("Failed to install clang.");
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
            Leaf::Logger::error("Failed to install cmake.");
    }

    if (!exists({"ninja", "--version"}))
    {
        fmt::println("Installing ninja...");
        const bool ninja_ok = tryCommands({{"winget", "install", "--id", "Ninja-build.Ninja", "-e"},
                                           {"choco", "install", "ninja", "-y"},
                                           {"scoop", "install", "ninja"}});
        setup_ok &= ninja_ok;
        if (!ninja_ok)
            Leaf::Logger::error("Failed to install ninja.");
    }

    if (!exists({"conan", "--version"}))
    {
        fmt::println("Installing conan...");
        const bool conan_ok = tryCommands({{"winget", "install", "-e", "--id", "JFrog.Conan"},
                                           {"python", "-m", "pip", "install", "--user", "conan"},
                                           {"pip", "install", "--user", "conan"}});
        setup_ok &= conan_ok;
        if (!conan_ok)
            Leaf::Logger::error("Failed to install conan.");
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
            return true;
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

    //WTF  is going on hereeeeeeeeeeeeeeeee TODO : FIX THISSSSSS
#if defined(__APPLE__)
    if (!exists({"clang", "--version"}) && !exists({"xcrun", "clang", "--version"}))
    {
        fmt::println("Installing clang toolchain...");
        const bool clang_ok = (has_brew && run({"brew", "install", "llvm"}) == 0) ||
                              run({"xcode-select", "--install"}) == 0;
        setup_ok &= clang_ok;
        if (!clang_ok)
            Leaf::Logger::error("Failed to install clang toolchain.");
    }
#else
    if (!exists({"clang", "--version"}))
    {
        fmt::println("Installing clang...");
        const bool clang_ok = installWithManager({"clang"});
        setup_ok &= clang_ok;
        if (!clang_ok)
            Leaf::Logger::error("Failed to install clang.");
    }
#endif

    if (!exists({"cmake", "--version"}))
    {
        fmt::println("Installing cmake...");
        const bool cmake_ok = installWithManager({"cmake"});
        setup_ok &= cmake_ok;
        if (!cmake_ok)
            Leaf::Logger::error("Failed to install cmake.");
    }

    if (!exists({"ninja", "--version"}))
    {
        fmt::println("Installing ninja...");
        bool ninja_ok = false;
        if (has_apt || has_dnf)
            ninja_ok = installWithManager({"ninja-build"});
        else
            ninja_ok = installWithManager({"ninja"});
        setup_ok &= ninja_ok;
        if (!ninja_ok)
            Leaf::Logger::error("Failed to install ninja.");
    }

    if (!exists({"conan", "--version"}))
    {
        fmt::println("Installing conan...");
        bool conan_ok = false;
        if (has_brew)
            conan_ok = run({"brew", "install", "conan"}) == 0;
        conan_ok = conan_ok || tryCommands({{"pip3", "install", "--user", "conan"},
                                            {"python3", "-m", "pip", "install", "--user", "conan"},
                                            {"pip", "install", "--user", "conan"}});
        setup_ok &= conan_ok;
        if (!conan_ok)
            Leaf::Logger::error("Failed to install conan.");
    }
#endif

    this->generateProfile();
    if (!setup_ok)
    {
        Leaf::Logger::warn("Toolchain setup completed with one or more install failures.");
    }
    else
    {
        Leaf::Logger::success("Toolchain setup completed successfully.");
    }
    Utils::printLeafConfigPath();
    return setup_ok ? 0 : 1;
}

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
                cpp_std = match[1].str();
            if (std::regex_search(line, match, comp_regex))
                compiler = match[1].str();
            if (std::regex_search(line, match, build_regex))
                build_type = match[1].str();
        }
        cmake.close();
    }

    if (EasyProc::ProcessHandler::runExternalProcess(
            {"conan", "profile", "detect", "--force"}, true, false) != 0)
    {
        Leaf::Logger::error("Conan profile detect failed. Ensure Conan is installed.");
        return 1;
    }

    if (EasyProc::ProcessHandler::runExternalProcess(
            {"conan", "profile", "path", "default"}, true, false) != 0)
    {
        Leaf::Logger::error("Failed to get conan profile path.");
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
        Leaf::Logger::error("Could not open default conan profile at: " + profile_path);
        return 1;
    }

    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(default_profile, line))
    {
        if (line.find("compiler.cppstd=") != std::string::npos)
            lines.push_back("compiler.cppstd=" + cpp_std);
        else if (line.find("build_type=") != std::string::npos)
            lines.push_back("build_type=" + build_type);
        else
            lines.push_back(line);
    }
    default_profile.close();

    // Add compiler override — check for duplicates first
    auto hasLine = [&](const std::string& prefix) -> bool
    {
        for (const auto& l : lines)
            if (l.find(prefix) != std::string::npos)
                return true;
        return false;
    };

    if (!hasLine("&:compiler="))
        lines.push_back("&:compiler=clang");

    if (EasyProc::ProcessHandler::runExternalProcess({"clang", "-v"}) != 0)
    {
        Logger::error("Failed to get clang compiler info.");
        return 1;
    }

    //TODO  append [conf] options for setting build_app directly in profiles
    //TODO and use modified profile(temp generated from profile) just remove [confi] options
    //TODO from temp profile generated when creating package and delete (or store in .profiles directory) them after package published

    std::string              log{EasyProc::ProcessHandler::getLog()};
    std::vector<std::string> clang_logs_lines{};
    pystring::splitlines(log, clang_logs_lines);

    if (!clang_logs_lines.empty())
    {
        std::string first_line = clang_logs_lines.front();
        std::regex  version_regex(R"(clang version ([0-9]+(\.[0-9]+)*))");
        std::smatch match;
        if (std::regex_search(first_line, match, version_regex))
        {
            std::string clang_compiler_version = match[1];
            if (!hasLine("&:compiler.version="))
            {
                lines.push_back(fmt::format("&:compiler.version={}",
                                            pystring::split(clang_compiler_version, ".").front()));
            }
        }
        else
        {
            Logger::warn("Failed to parse clang version.");
        }
    }

    std::string os_profile;
#ifdef _WIN32
    os_profile = "profiles/windows_profile";
#elif defined(__APPLE__)
    os_profile = "profiles/macos_profile";
    if (!hasLine("[conf]"))
    {
        lines.push_back("[conf]");
        lines.push_back("tools.system.package_manager:mode=install");
        lines.push_back("tools.system.package_manager:sudo=True");
    }
#else
    os_profile = "profiles/linux_profile";
    if (!hasLine("[conf]"))
    {
        lines.push_back("[conf]");
        lines.push_back("tools.system.package_manager:mode=install");
        lines.push_back("tools.system.package_manager:sudo=True");
    }
#endif

    std::filesystem::create_directories("profiles");
    std::ofstream out(os_profile);
    if (!out.is_open())
    {
        Leaf::Logger::error("Could not open output profile file for writing: " + os_profile);
        return 1;
    }
    for (const auto& l : lines)
        out << l << "\n";
    out.close();

    return 0;
}

} // namespace Leaf
