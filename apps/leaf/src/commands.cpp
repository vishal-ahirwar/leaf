#include "commands.h"
#include <vector>
#include <string>
#include <utils.h>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/color.h>
#include <iostream>
#include <algorithm>
#include<fstream>

int install()
{
  std::vector<std::string> conanInstallArgs{"conan", "install", ".", "-of", ".install", "-c", "tools.cmake.cmaketoolchain:user_presets=''"};
  runExternalProcess(conanInstallArgs);
  return 0;
};

int create()
{
  namespace fs = std::filesystem;
  fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "What would you like to name your project? ");
  std::string project_name{};
  std::getline(std::cin, project_name);
  if (std::ranges::any_of(project_name, [](const char c){ return c == ' '; }))
  {
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::red), "Error, Project name can't have space\n");
    return 1;
  };
  try
  {
    fs::create_directories(fs::path(project_name) / "apps" / project_name / "src");
    fs::create_directories(fs::path(project_name) / "cmake");
    fs::create_directories(fs::path(project_name) / "res");

    std::vector<std::string> files{fs::path(project_name) / "apps" / project_name / "src" / "main.cpp", fs::path(project_name) / "apps" / project_name / "CMakeLists.txt", fs::path(project_name) / "CMakeLists.txt", fs::path(project_name) / "conanfile.py", fs::path(project_name) / ".clangd", fs::path(project_name) / ".clang-format", fs::path(project_name) / ".gitignore"};

    std::ranges::for_each(files,[](const std::string&file){std::ofstream out{file};});

  }
  catch (const std::exception &e)
  {
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::red), "Exception : {}\n", e.what());
  }

  return 0;
};

int publish()
{
  runExternalProcess({"conan", "create", "."});
  return 0;
};

int upload()
{
  return 0;
};

int runTests()
{
  runExternalProcess({"ctest -B build"});
  return 0;
};

int format()
{

  return 0;
};

int clean()
{
  runExternalProcess({"cmake", "-B", "build", "--fresh"});
  return 0;
};

int debug()
{

  return 0;
};

int addPackage()
{
  return 0;
};

int removePackage()
{
  return 0;
};

int addApp()
{
  return 0;
};

int addLib()
{
  return 0;
};

int doctor()
{
  return 0;
};

void betterArgs(std::vector<std::string> &args, size_t argc, char **argv)
{
  for (size_t i = 0; i < argc; i++)
  {
    args.push_back(argv[i]);
  }
}


int build()
{
    runExternalProcess({"cmake","--build","build"});
    return 0;
}

int compile()
{
    return 0;
}

int run()
{
    return 0;
}

int init()
{
    return 0;
}
