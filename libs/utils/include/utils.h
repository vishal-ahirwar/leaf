#pragma once
#include <string>
#include <vector>

namespace Utils
{
bool replaceString(std::string& str, const std::string& from, const std::string& to);
std::vector<std::string> betterArgs(size_t argc, char** argv);
void                     printLeafConfigPath();
int                      generateProfiles();
std::string              getOSProfilePath();
std::string              trim(const std::string& s);
bool                     isValidProjectName(const std::string& name);
} // namespace Utils
