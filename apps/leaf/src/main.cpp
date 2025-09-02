
#include "commands.h"

#include <map>
#include <string>
#include <functional>
#include <ranges>
#include <algorithm>
#include <fmt/core.h>
#include <fmt/color.h>
#include <leafconfig.h>

using Command = std::pair<std::string, std::function<int()>>;
std::unordered_map<std::string, Command> commands{};

int help();

int main(int argc, char **argv)
{

  commands["create"] = {"Generates a new, fully structured Leaf project in a new directory.", create};
  commands["clean"] = {"Deletes all build files and temporary artifacts from the project directory.", clean};
  commands["format"] = {"Automatically formats all C++ source code files according to a consistent style.", format};
  commands["install"] = {"Fetches and installs all the dependencies listed in the conanfile.py.", install};
  commands["help"] = {"Displays a list of all available commands and their descriptions.", help};
  commands["build"] = {"A comprehensive command to compile the entire project and its dependencies and run the app.", build};
  commands["compile"] = {"Builds all targets in the project without running them.", compile};
  commands["run"] = {"Compiles and executes the main application target of your project.", run};
  commands["publish"] = {"Formally uploads the final, release-ready version of a package to a remote.", publish};
  commands["upload"] = {"Uploads a packaged library to a specified remote Conan repository.", upload};
  commands["doctor"] = {"Checks your system to ensure all required tools (Clang, CMake, Conan) are correctly installed.", doctor};
  commands["init"] = {"Initializes a new Leaf project structure within an existing directory.", init};

  if (argc < 2)
  {
    fmt::print("🍃 Leaf - A modern C++ project manager.\n");
    fmt::print("Run 'leaf help' for a list of commands.\n");
    return 0;
  }

  std::vector<std::string> args{};
  betterArgs(args, argc, argv);
  std::ranges::for_each(args,
    [&](const auto &command)
    {
      if(auto function=commands.find(command);function!=commands.end())
      {
        function->second.second();
      }
    });

  return 0;
}

int help()
{
  fmt::print("\n🍃 Leaf - v{} by {}\n", Project::VERSION_STRING, Project::COMPANY_NAME);
  fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green), "A modern, fast, and intuitive project/package manager for C++\n\n");
  std::map<std::string, Command> sorted_commands{commands.begin(), commands.end()};
  fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::medium_spring_green), "Available Commands\n");
  std::ranges::for_each(sorted_commands,
    [](const auto &command)
    {
      fmt::print("{} - ", command.first);
      fmt::print(fmt::emphasis::faint|fmt::fg(fmt::color::floral_white),"{}\n",command.second.first);
    });
  fmt::println("");
  return 0;
}