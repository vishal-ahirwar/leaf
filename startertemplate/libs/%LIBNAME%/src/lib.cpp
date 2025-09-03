#include "lib.h"
#include <fmt/core.h>

std::string get_greet(const std::string& name) {
    return fmt::format("Hello, {}!", name);
}
