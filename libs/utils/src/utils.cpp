#include "utils.h"
#include<fmt/core.h>
#include<fmt/color.h>
#include<iterator>
#include<iostream>
#include<ranges>
#include<algorithm>
int runExternalProcess(const std::vector<std::string> &args)
{
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "Executing ");
    std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
    std::cout << std::endl;
    return 0;
}