#include "logger.h"

#include <fmt/color.h>
#include <fmt/core.h>

namespace Leaf
{
void Logger::log(const std::string& name)
{
    fmt::print(fmt::emphasis::bold | fmt::emphasis::faint, "Message : ");
    fmt::println("{}", name);
}
} // namespace Leaf
