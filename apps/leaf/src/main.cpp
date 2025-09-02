
#include "commands.h"

#include <map>
#include <string>
#include <functional>
#include <ranges>
#include <algorithm>
#include <fmt/core.h>
#include <fmt/color.h>
#include <leafconfig.h>

std::unordered_map<std::string, std::function<int()>> commands{};

int main(int argc, char **argv)
{
  commands["create"] = create;
  commands["clean"] = clean;
  commands["format"] = format;
  commands["install"] = install;
  commands["help"] = help;
  if (argc < 2)
  {

    fmt::print("🍃 Leaf ");
    fmt::print(fmt::emphasis::bold | fmt::emphasis::faint | fmt::fg(fmt::color::light_green), "v{} by {}\nA modern, fast, and intuitive project/package manager for C++\n\n", Project::VERSION_STRING, Project::COMPANY_NAME);
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "Available Commands\n");
    std::ranges::for_each(commands, [](const auto &command)
                          { fmt::print("{} - \n", command.first); });
  }

  std::vector<std::string> args{};
  betterArgs(args, argc, argv);

  std::ranges::for_each(args, [&](const auto &command)
                        {if(auto function=commands.find(command);function!=commands.end()){
    function->second();
  } });

  return 0;
}