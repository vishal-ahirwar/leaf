#include "utils.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

bool replaceString(std::string& str, const std::string& from, const std::string& to)
{
    if (str.empty())
    {
        return false;
    };
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return true;
}

std::vector<std::string> betterArgs(size_t argc, char** argv)
{
    std::vector<std::string> args{};
    for (size_t i = 0; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    return args;
}