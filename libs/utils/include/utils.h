#pragma once
#include <string>
#include <vector>
int runExternalProcess(const std::vector<std::string>& args);

bool replaceString(std::string& str, const std::string& from, std::string& to);