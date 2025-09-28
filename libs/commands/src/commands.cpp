#include "../include/commands.h"

#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>

#include <algorithm>
#include <string>


int Commands::exec()
{
    if (_args.size() < 2)
    {
        _default();
        return 1;
    }

    std::for_each(_args.begin() + 1,
                  _args.end(),
                  [this](const auto& command)
                  {
                      if (const auto& result = _commands.find(command); result != _commands.end())
                      {
                          result->second.second();
                      }
                  });
    return 0;
}

void Commands::registerCommands(std::string&&command,std::string&&description,std::function<int(void)>callable)
{
    _commands[command]={description,callable};
};

const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&Commands::getCommands()const
{
    return _commands;
}

const  std::vector<std::string>& Commands::getArgs()const{
    return _args;
}