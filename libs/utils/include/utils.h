#pragma once
#include <string>
#include <vector>

namespace Leaf
{
bool replaceString(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> betterArgs(size_t argc, char** argv);
void                     printLeafConfigPath();
int                      generateProfiles();
} // namespace Leaf
