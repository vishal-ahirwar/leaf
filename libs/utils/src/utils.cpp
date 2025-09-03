#include "utils.h"

#include <fmt/color.h>
#include <fmt/core.h>
#include <spinner.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <reproc++/run.hpp>
#include <string>

int runExternalProcess(const std::vector<std::string>& args)
{
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "Executing ");
    std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
    std::cout << std::endl;
    reproc::process child;
    reproc::options options;
    options.redirect.parent = true;
    auto ec                 = child.start(args, options);
    return ec ? ec.value() : 0;
}

bool replaceString(std::string& str, const std::string& from, std::string& to)
{
    if (str.empty()) {
        return false;
    };
    size_t start_pos=0;
    while ((start_pos=str.find(from,start_pos))!=std::string::npos) {
        str.replace(start_pos,from.length(),to);
        start_pos+=to.length();
    }
    return true;
}