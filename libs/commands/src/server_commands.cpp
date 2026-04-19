//
// Server commands: init, start, push, add
// Provides a built-in Conan V2 compatible package server.
// No JFrog, no conan_server, no pip installs — just Leaf standalone binary.
//

#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <sago/platform_folders.h>
#include <utils.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "commands.h"
#include "logger.h"
#include "server.h"

namespace Leaf
{

namespace
{

std::filesystem::path getServerDir()
{
    return std::filesystem::path(sago::getConfigHome()) / ".leaf" / "server";
}

} // namespace

int CLI::server()
{
    const auto& positionals = _commands->getPositionals();

    if (positionals.empty())
    {
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green),
                   "\n🍃 Leaf Package Server\n\n");
        fmt::println("Usage: leaf server <subcommand>\n");
        fmt::println("Subcommands:");
        fmt::println("  init              Initialize the server data directory");
        fmt::println("  start [port]      Start the package server (default: 9300)");
        fmt::println("  push  <pattern>   Upload packages to a Leaf server remote");
        fmt::println("  add   <name> <url> Add a remote Leaf server to Conan config");
        fmt::println("");
        fmt::println("Quick start:");
        fmt::println("  leaf server init");
        fmt::println("  leaf server start");
        fmt::println("  # On another machine:");
        fmt::println("  leaf server add my-server http://<server-ip>:9300");
        fmt::println("  leaf addpkg mylib");
        fmt::println("");
        return 0;
    }

    const std::string subcmd = positionals.front();
    namespace fs             = std::filesystem;
    auto server_dir          = getServerDir();

    if (subcmd == "init")
    {
        fs::create_directories(server_dir / "data");
        Leaf::Logger::success("Server initialized at: " + server_dir.string());
        Leaf::Logger::info("Run 'leaf server start' to launch the server.");
        return 0;
    }

    if (subcmd == "start")
    {

        auto data_path = server_dir / "data";

        // Parse port: `leaf server start 9300` or `leaf server start --port 9300`
        std::string port = "9300";
        if (auto port_opt = _commands->getOptionValue("port"); port_opt.has_value())
        {
            port = *port_opt;
        }
        else if (positionals.size() > 1)
        {
            port = positionals[1];
        }
        auto               argc = _args.size();
        std::vector<char*> argv;
        std::for_each(_args.begin(),
                      _args.end(),
                      [&argv](const std::string& arg)
                      { argv.push_back(const_cast<char*>(arg.c_str())); });
        argv.push_back(nullptr);
        return server::run(argc, argv.data());
    }

    if (subcmd == "push")
    {
        if (positionals.size() < 2)
        {
            fmt::println("Usage: leaf server push <pattern> [-r <remote>]");
            fmt::println("Example: leaf server push 'mylib/*'");
            fmt::println("         leaf server push '*' -r my-team-server");
            return 1;
        }

        std::string pattern = positionals[1];
        std::string remote  = "leaf-server";

        for (size_t i = 0; i < _args.size(); ++i)
        {
            if ((_args[i] == "-r" || _args[i] == "--remote") && i + 1 < _args.size())
            {
                remote = _args[i + 1];
            }
        }

        Leaf::Logger::info(fmt::format("Uploading '{}' to remote '{}'...", pattern, remote));
        int result = EasyProc::ProcessHandler::runExternalProcess(
            {"conan", "upload", pattern, "-r", remote, "-c"}, false, true);

        if (result != 0)
        {
            Leaf::Logger::error("Upload failed.");
            Leaf::Logger::info("Make sure the server is running and the remote is configured:");
            Leaf::Logger::info(fmt::format("  leaf server add {} http://<server>:<port>", remote));
            return 1;
        }

        Leaf::Logger::success("Packages uploaded successfully.");
        return 0;
    }

    // ─── add ───
    if (subcmd == "add")
    {
        if (positionals.size() < 3)
        {
            fmt::println("Usage: leaf server add <name> <url>");
            fmt::println("Example: leaf server add my-server http://192.168.1.100:9300");
            return 1;
        }

        std::string name = positionals[1];
        std::string url  = positionals[2];

        int result = EasyProc::ProcessHandler::runExternalProcess(
            {"conan", "remote", "add", name, url}, false, true);

        if (result != 0)
        {
            // Remote might already exist — try updating its URL
            result = EasyProc::ProcessHandler::runExternalProcess(
                {"conan", "remote", "update", name, "--url", url}, false, true);
        }

        if (result == 0)
        {
            Leaf::Logger::success(fmt::format("Remote '{}' configured: {}", name, url));
        }
        else
        {
            Leaf::Logger::error("Failed to configure remote.");
            return 1;
        }
        return 0;
    }

    Leaf::Logger::error(
        fmt::format("Unknown subcommand: '{}'. Run 'leaf server' for help.", subcmd));
    return 1;
}

} // namespace Leaf
