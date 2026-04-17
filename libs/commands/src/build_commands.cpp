//
// Build, compile, run, and clean command implementations
//

#include "commands.h"

#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <spinner.h>
#include <utils.h>

#include <filesystem>
#include <optional>

#include "logger.h"

namespace Leaf
{

int CLI::build()
{
    namespace fs                   = std::filesystem;
    const bool        release_mode = isReleaseMode();
    const std::string mode         = release_mode ? "release" : "debug";
    const std::string build_dir    = fmt::format(".build/{}", mode);
    std::string       app_target{};
    if (const auto target = getTargetOption(); target.has_value())
    {
        app_target = *target;
    }
    else if (const auto app = getAppOption(); app.has_value())
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
    if (const auto target = getTargetOption(); target.has_value())
    {
        appName = *target;
    }
    else if (const auto app = getAppOption(); app.has_value())
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

    const std::string mode = isReleaseMode() ? "release" : "debug";
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

int CLI::clean()
{
    namespace fs = std::filesystem;
    Spinner spin("Running clean build");
    spin.start();
    if (isReleaseMode())
    {
        if (!fs::exists(".install/Release"))
            install();
        if (0 != EasyProc::ProcessHandler::runExternalProcess(
                     {"cmake", "--preset", "release", "--fresh"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        }
    }
    else
    {
        if (!fs::exists(".install/Debug"))
            install();
        if (0 !=
            EasyProc::ProcessHandler::runExternalProcess({"cmake", "--preset", "debug", "--fresh"}))
        {
            fmt::println("{}", EasyProc::ProcessHandler::getLog());
        }
    }
    spin.stop();
    return 0;
}

} // namespace Leaf
