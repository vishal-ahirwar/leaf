#include<fmt/core.h>
#include<fmt/color.h>
#include<vector>
#include <algorithm>
#include <ranges>
#include<reproc++/run.hpp>
#include <iostream>
#include <iterator>
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