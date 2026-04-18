

#include "commands.h"

#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <spinner.h>
#include <utils.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "logger.h"

namespace Leaf
{


int CLI::search()
{
    const auto& positionals = _commands->getPositionals();
    if (positionals.empty())
    {
        fmt::println("Usage: leaf search <query>");
        fmt::println("Example: leaf search fmt");
        fmt::println("         leaf search boost*");
        return 1;
    }

    const std::string query = positionals.front();
    const std::string search_pattern =
        query.find('*') != std::string::npos ? query : query + "*";

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "\n🔍 Searching for '{}' ...\n\n",
               query);

    Spinner spin("Searching packages");
    spin.start();
    int result = EasyProc::ProcessHandler::runExternalProcess(
        {"conan", "search", search_pattern});
    spin.stop();

    if (result != 0)
    {
        std::string log = EasyProc::ProcessHandler::getLog();
        if (log.find("ERROR") != std::string::npos || log.find("No remote") != std::string::npos)
        {
            Leaf::Logger::warn("Search failed. Output:");
            fmt::println("{}", log);
        }
        else
        {
            fmt::println("No packages found matching '{}'.", query);
        }
        return 1;
    }

    std::string log = EasyProc::ProcessHandler::getLog();
    if (log.empty())
    {
        fmt::println("No packages found matching '{}'.", query);
        return 0;
    }

    fmt::print(fmt::emphasis::bold, "📦 Results:\n\n");
    std::istringstream stream(log);
    std::string        line;
    int                count = 0;
    while (std::getline(stream, line))
    {
        std::string trimmed = Utils::trim(line);
        if (trimmed.empty())
            continue;
        // Package lines contain a slash (e.g. "fmt/11.2.0")
        if (trimmed.find('/') != std::string::npos && trimmed.find("remote") == std::string::npos)
        {
            fmt::print(fmt::fg(fmt::color::light_green), "  • {}\n", trimmed);
            count++;
        }
        else
        {
            fmt::println("  {}", trimmed);
        }
    }

    if (count > 0)
    {
        fmt::print(fmt::emphasis::faint, "\n  {} package(s) found.\n", count);
        fmt::println("  Use 'leaf info <package>' for more details.");
        fmt::println("  Use 'leaf addpkg <package>' to add to your project.\n");
    }

    return 0;
}


int CLI::info()
{
    const auto& positionals = _commands->getPositionals();
    if (positionals.empty())
    {
        fmt::println("Usage: leaf info <package>");
        fmt::println("Example: leaf info fmt");
        fmt::println("         leaf info fmt/11.2.0");
        return 1;
    }

    std::string package = positionals.front();

    // If no version specified, try to discover the latest version
    if (package.find('/') == std::string::npos)
    {
        if (EasyProc::ProcessHandler::runExternalProcess(
                {"conan", "search", package + "/*"}) == 0)
        {
            std::string log = EasyProc::ProcessHandler::getLog();
            std::regex  version_regex(package + R"(/([^\s]+))");
            std::string last_version;
            std::sregex_iterator it(log.begin(), log.end(), version_regex);
            std::sregex_iterator end;
            while (it != end)
            {
                last_version = it->str();
                ++it;
            }
            if (!last_version.empty())
                package = last_version;
            else
            {
                Leaf::Logger::error(
                    fmt::format("Could not find any version for '{}'.", positionals.front()));
                return 1;
            }
        }
        else
        {
            Leaf::Logger::error(
                fmt::format("Package '{}' not found in remotes.", positionals.front()));
            return 1;
        }
    }

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "\n📋 Package Information: {}\n\n",
               package);

    // Use `conan graph info --requires=<ref>` — the correct Conan 2 approach
    Spinner spin("Fetching package info");
    spin.start();
    int result = EasyProc::ProcessHandler::runExternalProcess(
        {"conan", "graph", "info", fmt::format("--requires={}", package)});
    spin.stop();

    if (result == 0)
    {
        std::string        log = EasyProc::ProcessHandler::getLog();
        std::istringstream stream(log);
        std::string        line;

        while (std::getline(stream, line))
        {
            std::string trimmed = Utils::trim(line);
            if (trimmed.empty())
                continue;

            // Highlight keys (lines with colons)
            auto colon_pos = trimmed.find(':');
            if (colon_pos != std::string::npos && colon_pos < 30)
            {
                std::string key   = trimmed.substr(0, colon_pos);
                std::string value = trimmed.substr(colon_pos);
                fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_steel_blue),
                           "  {}", key);
                fmt::println("{}", value);
            }
            else if (trimmed.find('/') != std::string::npos)
            {
                fmt::print(fmt::fg(fmt::color::light_green), "  {}\n", trimmed);
            }
            else
            {
                fmt::println("  {}", trimmed);
            }
        }
    }
    else
    {
        Leaf::Logger::error(fmt::format("Could not fetch info for '{}'.", package));
        std::string log = EasyProc::ProcessHandler::getLog();
        if (!log.empty())
            fmt::println("{}", log);
        return 1;
    }

    fmt::println("");
    return 0;
}


int CLI::depTree()
{
    namespace fs = std::filesystem;

    if (!fs::exists("conanfile.py") && !fs::exists("conanfile.txt"))
    {
        Leaf::Logger::error("No conanfile.py or conanfile.txt found in current directory.");
        Leaf::Logger::info("Run 'leaf create <name>' or 'leaf init' to create a project first.");
        return 1;
    }

    if (!fs::exists(Utils::getOSProfilePath()))
    {
        Leaf::Logger::info("Profile not found. Generating...");
        generateProfile();
    }

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
               "\n🌳 Dependency Tree\n\n");

   
    fmt::print(fmt::emphasis::bold, "Direct dependencies (from conanfile.py):\n");
    {
        std::ifstream conanfile("conanfile.py");
        if (conanfile.is_open())
        {
            std::string line;
            std::regex  req_regex(R"(self\.requires\s*\(\s*\"([^\"]+)\"\s*\))");
            std::regex  test_req_regex(R"(self\.test_requires\s*\(\s*\"([^\"]+)\"\s*\))");
            bool        found = false;
            while (std::getline(conanfile, line))
            {
                std::smatch match;
                if (std::regex_search(line, match, req_regex))
                {
                    fmt::print(fmt::fg(fmt::color::light_green), "  ├── {}\n", match[1].str());
                    found = true;
                }
                else if (std::regex_search(line, match, test_req_regex))
                {
                    fmt::print(fmt::fg(fmt::color::light_blue), "  ├── {} (test)\n",
                               match[1].str());
                    found = true;
                }
            }
            if (!found)
                fmt::println("  (no dependencies found)");
        }
    }

  
    fmt::print(fmt::emphasis::bold, "\nFull dependency graph:\n");

    Spinner spin("Resolving dependency graph");
    spin.start();
    int result = EasyProc::ProcessHandler::runExternalProcess(
        {"conan", "graph", "info", ".", "-pr", Utils::getOSProfilePath()});
    spin.stop();

    if (result == 0)
    {
        std::string        log = EasyProc::ProcessHandler::getLog();
        std::istringstream stream(log);
        std::string        line;
        while (std::getline(stream, line))
        {
            std::string trimmed = Utils::trim(line);
            if (trimmed.empty())
                continue;
            if (trimmed.find('/') != std::string::npos && !trimmed.starts_with("="))
            {
                fmt::print(fmt::fg(fmt::color::light_green), "  {}\n", trimmed);
            }
            else if (trimmed.starts_with("Requires:") || trimmed.starts_with("Required by:"))
            {
                fmt::print(fmt::emphasis::bold, "  {}\n", trimmed);
            }
            else
            {
                fmt::println("  {}", trimmed);
            }
        }
    }
    else
    {
        Leaf::Logger::warn("Could not resolve full dependency graph.");
        Leaf::Logger::info("Try running 'leaf install' first to fetch dependencies.");
        std::string log = EasyProc::ProcessHandler::getLog();
        if (!log.empty())
            fmt::println("{}", log);
    }

    fmt::println("");
    return 0;
}

} // namespace Leaf
