#include "logger.h"

#include <fmt/color.h>
#include <fmt/core.h>

namespace
{
// Clears any spinner/progress text on the current terminal line before printing.
// Uses ANSI escape: \r (carriage return) + \033[2K (erase entire line).
// Supported on Windows 10+ (cmd, PowerShell, Terminal) and all Unix terminals.
// Harmless when no spinner is active.
void clearLine()
{
    fmt::print("\r\033[2K");
}
} // namespace

namespace Leaf
{
void Logger::log(const std::string& msg)
{
    clearLine();
    fmt::print(fmt::emphasis::bold | fmt::emphasis::faint, "Message : ");
    fmt::println("{}\n", msg);
}

void Logger::info(const std::string& msg)
{
    clearLine();
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::dodger_blue), "  info : ");
    fmt::println("{}", msg);
}

void Logger::warn(const std::string& msg)
{
    clearLine();
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::orange), "  warn : ");
    fmt::println("{}", msg);
}

void Logger::error(const std::string& msg)
{
    clearLine();
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::crimson), " error : ");
    fmt::println("{}", msg);
}

void Logger::success(const std::string& msg)
{
    clearLine();
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_sea_green), "    ok : ");
    fmt::println("{}", msg);
}

void Logger::debug(const std::string& msg)
{
    clearLine();
    fmt::print(fmt::emphasis::faint | fmt::fg(fmt::color::gray), " debug : ");
    fmt::println("{}", msg);
}
} // namespace Leaf
