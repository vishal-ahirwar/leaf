#include "utils.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>


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