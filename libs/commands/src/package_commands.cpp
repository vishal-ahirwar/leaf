//
// Package management: install, addPackage (REWRITTEN), removePackage, publish, upload
//

#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <progress.h>
#include <utils.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "commands.h"
#include "logger.h"

namespace Leaf
{

namespace
{

struct ConanCMakeInfo
{
    std::string              conan_name;      // e.g. "fmt"
    std::string              cmake_find_name; // for find_package()
    std::vector<std::string> cmake_targets;   // for target_link_libraries()
};

struct ProjectTarget
{
    std::string name;       // e.g. "commands"
    std::string type;       // "executable" or "library"
    std::string cmake_path; // e.g. "libs/commands/CMakeLists.txt"
};

std::vector<ConanCMakeInfo> resolveCMakeInfo(const std::vector<std::string>& pkg_base_names,
                                             bool                            release_mode)
{
    namespace fs = std::filesystem;
    std::vector<ConanCMakeInfo> results;

    const std::string gen_dir =
        release_mode ? ".install/build/Release/generators" : ".install/build/Debug/generators";

    for (const auto& pkg : pkg_base_names)
    {
        ConanCMakeInfo info;
        info.conan_name = pkg;

        std::string pkg_lower = pkg;
        std::transform(pkg_lower.begin(), pkg_lower.end(), pkg_lower.begin(), ::tolower);

        // --- Step 1: find the cmake config file to determine the find_package() name ---
        bool found_config = false;
        if (fs::exists(gen_dir))
        {
            for (const auto& entry : fs::directory_iterator(gen_dir))
            {
                if (!entry.is_regular_file())
                    continue;
                std::string fname       = entry.path().filename().string();
                std::string fname_lower = fname;
                std::transform(
                    fname_lower.begin(), fname_lower.end(), fname_lower.begin(), ::tolower);

                if (fname_lower.find("version") != std::string::npos)
                    continue;

                if (fname_lower.find(pkg_lower) != std::string::npos &&
                    fname_lower.find("config") != std::string::npos &&
                    fname_lower.ends_with(".cmake"))
                {
                    // "fmt-config.cmake" → "fmt"   |   "fmtConfig.cmake" → "fmt"
                    size_t dash_pos = fname.find("-config.");
                    if (dash_pos != std::string::npos)
                    {
                        info.cmake_find_name = fname.substr(0, dash_pos);
                    }
                    else
                    {
                        size_t config_pos = fname.find("Config.");
                        if (config_pos != std::string::npos)
                        {
                            info.cmake_find_name = fname.substr(0, config_pos);
                        }
                    }
                    found_config = true;
                    break;
                }
            }
        }

        if (!found_config || info.cmake_find_name.empty())
        {
            info.cmake_find_name = pkg; // fallback
        }

        // --- Step 2: parse *Targets.cmake to extract actual cmake target names ---
        info.cmake_targets.clear();
        if (fs::exists(gen_dir))
        {
            for (const auto& entry : fs::directory_iterator(gen_dir))
            {
                if (!entry.is_regular_file())
                    continue;
                std::string fname       = entry.path().filename().string();
                std::string fname_lower = fname;
                std::transform(
                    fname_lower.begin(), fname_lower.end(), fname_lower.begin(), ::tolower);

                // Match main Targets file but NOT *Targets-debug.cmake / *Targets-release.cmake
                if (fname_lower.find(pkg_lower) != std::string::npos &&
                    fname_lower.find("targets") != std::string::npos &&
                    fname_lower.ends_with(".cmake") &&
                    fname_lower.find("debug") == std::string::npos &&
                    fname_lower.find("release") == std::string::npos)
                {
                    std::ifstream targets_file(entry.path());
                    std::string   line;
                    std::regex    target_regex(R"(add_library\(\s*(\S+::\S+))");
                    while (std::getline(targets_file, line))
                    {
                        std::smatch match;
                        std::string search_line = line;
                        while (std::regex_search(search_line, match, target_regex))
                        {
                            std::string t = match[1].str();
                            if (std::find(info.cmake_targets.begin(),
                                          info.cmake_targets.end(),
                                          t) == info.cmake_targets.end())
                            {
                                info.cmake_targets.push_back(t);
                            }
                            search_line = match.suffix();
                        }
                    }
                }
            }
        }

        // Fallback: convention pkg::pkg
        if (info.cmake_targets.empty())
        {
            info.cmake_targets.push_back(info.cmake_find_name + "::" + info.cmake_find_name);
        }

        results.push_back(info);
    }
    return results;
}

std::vector<ProjectTarget> discoverProjectTargets()
{
    namespace fs = std::filesystem;
    std::vector<ProjectTarget> targets;

    auto scanDir = [&](const std::string& base_dir)
    {
        if (!fs::exists(base_dir))
            return;
        for (const auto& entry : fs::directory_iterator(base_dir))
        {
            if (!entry.is_directory())
                continue;
            auto cmake_path = entry.path() / "CMakeLists.txt";
            if (!fs::exists(cmake_path))
                continue;

            std::ifstream file(cmake_path.string());
            std::string   content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());

            std::regex  exe_regex(R"(add_executable\s*\(\s*(\w+))");
            std::regex  lib_regex(R"(add_library\s*\(\s*(\w+))");
            std::smatch match;

            if (std::regex_search(content, match, exe_regex))
            {
                targets.push_back({match[1].str(), "executable", cmake_path.string()});
            }
            else if (std::regex_search(content, match, lib_regex))
            {
                targets.push_back({match[1].str(), "library", cmake_path.string()});
            }
        }
    };

    scanDir("apps");
    scanDir("libs");
    return targets;
}

void addFindPackageToFile(const std::string& cmake_path, const std::string& cmake_find_name)
{
    std::ifstream            in(cmake_path);
    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(in, line))
        lines.push_back(line);
    in.close();

    // Already present?
    for (const auto& l : lines)
    {
        if (l.find("find_package(" + cmake_find_name) != std::string::npos)
            return;
    }

    // Insert after the last existing find_package, or at the top after comments
    size_t insert_pos = 0;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        std::string trimmed = Utils::trim(lines[i]);
        if (trimmed.empty() || trimmed[0] == '#')
        {
            insert_pos = i + 1;
            continue;
        }
        if (trimmed.find("find_package") != std::string::npos)
        {
            insert_pos = i + 1;
        }
        else if (insert_pos > 0 && trimmed.find("find_package") == std::string::npos)
        {
            break;
        }
    }

    lines.insert(lines.begin() + static_cast<long>(insert_pos),
                 "find_package(" + cmake_find_name + " REQUIRED)");

    std::ofstream out(cmake_path);
    for (const auto& l : lines)
        out << l << "\n";
}

void addTargetLinkToFile(const std::string&              cmake_path,
                         const std::string&              target_name,
                         const std::vector<std::string>& cmake_targets)
{
    std::ifstream            in(cmake_path);
    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(in, line))
        lines.push_back(line);
    in.close();
    bool append_target_link_lines = true;
    for (auto& l : lines)
    {
        if (l.find("target_link_libraries") != std::string::npos &&
            l.find(target_name) != std::string::npos)
        {
            append_target_link_lines = false;
            for (const auto& cmake_target : cmake_targets)
            {
                if (l.find(cmake_target) != std::string::npos)
                    continue; // already linked
                size_t close_paren = l.find_last_of(')');
                if (close_paren != std::string::npos)
                {
                    l.insert(close_paren, " " + cmake_target);
                }
            }
            break;
        }
    }
    if (append_target_link_lines)
    {
        lines.push_back(fmt::format("target_link_libraries({} PRIVATE {})",target_name,*cmake_targets.begin()));
    }
    std::ofstream out(cmake_path);
    for (const auto& l : lines)
        out << l << "\n";
}

} // namespace

int CLI::install()
{
    progress::Spinner spin("Installing dependencies");
    if (!isVerboseMode()) spin.start();

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
    if (isReleaseMode())
    {
        conanInstallArgs.emplace_back("build_type=Release");
    }
    else
    {
        conanInstallArgs.emplace_back("build_type=Debug");
    }
    if (0 != EasyProc::ProcessHandler::runExternalProcess(conanInstallArgs, !isVerboseMode(), isVerboseMode()))
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
    }
    if (!isVerboseMode()) spin.stop();
    return 0;
}

int CLI::addPackage()
{
    const auto& package_args = _commands->getPositionals();
    if (package_args.empty())
    {
        fmt::println("Usage: leaf addpkg <pkg1> [pkg2] ... [--release]");
        return 1;
    }

    std::vector<std::string> packages_to_install;
    std::vector<std::string> pkg_base_names;

    for (const auto& package : package_args)
    {
        const std::string search_query =
            package.find('/') == std::string::npos ? package + "/*" : package;
        if (EasyProc::ProcessHandler::runExternalProcess({"conan", "search", search_query}) != 0)
        {
            Leaf::Logger::error(
                fmt::format("package '{}' was not found in configured conan remotes.", package));
            continue;
        }
        const std::string normalized =
            package.find('/') == std::string::npos ? package + "/[>=0]" : package;
        packages_to_install.push_back(normalized);

        std::string base      = package;
        auto        slash_pos = base.find('/');
        if (slash_pos != std::string::npos)
            base = base.substr(0, slash_pos);
        pkg_base_names.push_back(base);
    }

    if (packages_to_install.empty())
        return 1;

    std::vector<std::string> conan_lines;
    {
        std::ifstream in("conanfile.py");
        if (!in.is_open())
        {
            Leaf::Logger::error("failed to open conanfile.py");
            return -1;
        }
        std::string line;
        while (std::getline(in, line))
            conan_lines.push_back(line);
    }

    auto it = std::find(conan_lines.begin(), conan_lines.end(), "    def requirements(self):");
    if (it == conan_lines.end())
        it = std::find(conan_lines.begin(), conan_lines.end(), "def requirements(self):");

    if (it != conan_lines.end())
    {
        for (const auto& pkg : packages_to_install)
        {
            it = conan_lines.insert(it + 1, fmt::format("        self.requires(\"{}\")", pkg));
        }
    }
    else
    {
        Leaf::Logger::error("'def requirements(self):' not found in conanfile.py");
        return -1;
    }

    {
        std::ofstream out("conanfile.py");
        if (!out.is_open())
        {
            Leaf::Logger::error("failed to write to conanfile.py");
            return -1;
        }
        for (const auto& line : conan_lines)
            out << line << std::endl;
    }

    Leaf::Logger::info("Installing dependencies...");
    if (install() != 0)
    {
        Leaf::Logger::error("Dependency installation failed.");
        return -1;
    }

    auto cmake_infos = resolveCMakeInfo(pkg_base_names, isReleaseMode());

    auto project_targets = discoverProjectTargets();

    if (project_targets.empty())
    {
        Leaf::Logger::warn("No build targets found in apps/ or libs/. "
                           "You will need to manually update your CMakeLists.txt files.");
        fmt::println("\nAdd these to your CMakeLists.txt:");
        for (const auto& info : cmake_infos)
        {
            fmt::println("  find_package({} REQUIRED)", info.cmake_find_name);
            for (const auto& t : info.cmake_targets)
                fmt::println("  # Link with: {}", t);
        }
        return 0;
    }

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "\n Found build targets:\n");
    for (size_t i = 0; i < project_targets.size(); ++i)
    {
        fmt::println("  [{}] {} ({}) — {}",
                     i + 1,
                     project_targets[i].name,
                     project_targets[i].type,
                     project_targets[i].cmake_path);
    }

    for (const auto& cmake_info : cmake_infos)
    {
        std::string targets_display;
        for (const auto& t : cmake_info.cmake_targets)
        {
            if (!targets_display.empty())
                targets_display += ", ";
            targets_display += t;
        }

        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
                   "\nWhich targets should link to {}? ",
                   targets_display);
        fmt::print("(comma-separated numbers, or 'all'): ");

        std::string input;
        std::getline(std::cin, input);
        input = Utils::trim(input);

        std::vector<size_t> selected;
        if (input == "all" || input == "a" || input.empty())
        {
            for (size_t i = 0; i < project_targets.size(); ++i)
                selected.push_back(i);
        }
        else
        {
            std::istringstream iss(input);
            std::string        token;
            while (std::getline(iss, token, ','))
            {
                token = Utils::trim(token);
                if (token.empty())
                    continue;
                try
                {
                    size_t idx = std::stoul(token) - 1;
                    if (idx < project_targets.size())
                        selected.push_back(idx);
                    else
                        Leaf::Logger::warn(fmt::format("Invalid index: {}", token));
                }
                catch (...)
                {
                    Leaf::Logger::warn(fmt::format("Invalid input: '{}'", token));
                }
            }
        }

        for (size_t idx : selected)
        {
            const auto& target = project_targets[idx];
            addFindPackageToFile(target.cmake_path, cmake_info.cmake_find_name);
            addTargetLinkToFile(target.cmake_path, target.name, cmake_info.cmake_targets);
            Leaf::Logger::success(
                fmt::format("Linked {} to target '{}'", targets_display, target.name));
        }
    }

    Leaf::Logger::success("Packages added successfully.");
    return 0;
}

int CLI::removePackage()
{
    const auto& package_args = _commands->getPositionals();
    if (package_args.empty())
    {
        fmt::println("Usage: leaf remove <pkg1> [pkg2] ...");
        return 1;
    }

    const std::vector<std::string>& packages_to_remove = package_args;

    // 1. Remove from conanfile.py
    std::vector<std::string> lines;
    {
        std::ifstream in("conanfile.py");
        if (!in.is_open())
        {
            Leaf::Logger::error("failed to open conanfile.py");
            return -1;
        }
        std::string line;
        while (std::getline(in, line))
        {
            bool remove = false;
            for (const auto& pkg : packages_to_remove)
            {
                if (line.find("self.requires") != std::string::npos &&
                    line.find("\"" + pkg) != std::string::npos)
                {
                    remove = true;
                    break;
                }
            }
            if (!remove)
                lines.push_back(line);
        }
    }

    {
        std::ofstream out("conanfile.py");
        if (!out.is_open())
        {
            Leaf::Logger::error("failed to write to conanfile.py");
            return -1;
        }
        for (const auto& l : lines)
            out << l << "\n";
    }

    if (install() != 0)
        return -1;

    auto updateCMake = [&](const std::string& path)
    {
        if (!std::filesystem::exists(path))
            return;
        std::vector<std::string> cmake_lines;
        std::ifstream            cmake_in(path);
        if (!cmake_in.is_open())
            return;
        std::string l;
        while (std::getline(cmake_in, l))
        {
            bool remove = false;
            if (l.find("find_package") != std::string::npos)
            {
                for (const auto& pkg : packages_to_remove)
                {
                    std::string clean_pkg = pkg.substr(0, pkg.find('/'));
                    if (l.find(clean_pkg) != std::string::npos)
                    {
                        remove = true;
                        break;
                    }
                }
            }
            if (!remove)
                cmake_lines.push_back(l);
        }
        cmake_in.close();

        std::ofstream cmake_out(path);
        for (const auto& line : cmake_lines)
            cmake_out << line << "\n";
    };

    updateCMake("libs/CMakeLists.txt");
    updateCMake("src/CMakeLists.txt");

    for (const auto& dir : {"apps", "libs"})
    {
        if (!std::filesystem::exists(dir))
            continue;
        for (const auto& entry : std::filesystem::directory_iterator(dir))
        {
            if (entry.is_directory())
                updateCMake(entry.path().string() + "/CMakeLists.txt");
        }
    }

    Leaf::Logger::success(
        "Packages removed. Please verify your CMakeLists.txt for leftover linking.");
    return 0;
}

int CLI::publish()
{
    progress::Spinner spin("Creating Package");
    if (!isVerboseMode()) spin.start();
    if (0 != EasyProc::ProcessHandler::runExternalProcess(
                 {"conan", "create", ".", "-b", "missing", "-pr", Utils::getOSProfilePath()},
                 !isVerboseMode(),
                 isVerboseMode()))
    {
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
    }
    if (!isVerboseMode()) spin.stop();
    return 0;
}

int CLI::upload()
{
    const auto& positionals = _commands->getPositionals();
    if (positionals.empty())
    {
        fmt::println("Use: leaf upload <package_pattern> -r <remote>");
    }

    std::string pattern{};
    std::string remote{};

    for (size_t i = 2; i < _args.size(); ++i)
    {
        if ((_args[i] == "-r" || _args[i] == "--remote") && _args.size() > i + 1)
        {
            remote = _args[i + 1];
        }
        if ((_args[i] == "-p" || _args[i] == "--pattern") && _args.size() > i + 1)
        {
            pattern = _args[i + 1];
        }
    }
    namespace fs = std::filesystem;
    if (remote.empty())
        remote = "leaf-server";
    if (pattern.empty())
        pattern = fs::current_path().filename().string();
    std::vector<std::string> conanArgs = {"conan", "upload", pattern, "-r", remote, "-c"};
    if (EasyProc::ProcessHandler::runExternalProcess(conanArgs, !isVerboseMode(), isVerboseMode()) != 0)
    {
        Leaf::Logger::error("Upload failed.");
        fmt::println("{}", EasyProc::ProcessHandler::getLog());
        return 1;
    }

    Leaf::Logger::success("Upload successful.");
    return 0;
}

} // namespace Leaf
