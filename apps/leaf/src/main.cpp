
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <leafconfig.h>

int main()
{
  std::cout << std::format("{} by {} v{}\n", Project::PROJECT_NAME, Project::COMPANY_NAME, Project::VERSION_STRING);
  return 0;
}