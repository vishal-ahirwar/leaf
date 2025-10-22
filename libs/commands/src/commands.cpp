#include <commands.h>

#include <fmt/base.h>
#include <fmt/color.h>


#include <algorithm>
#include <filesystem>

#include <string>
#include <vector>

namespace Leaf
{
int Commands::exec()
{
    if (_args.size() < 2)
    {
        _default();
        return 1;
    }
    bool commandExists{false};
    std::for_each(_args.begin() + 1,
                  _args.end(),
                  [this, &commandExists](const auto& command)
                  {
                      if (const auto& result = _commands.find(command); result != _commands.end())
                      {
                          commandExists = true;
                          result->second.second();
                      }else
                      {
                          fmt::print(fmt::fg(fmt::color::red), "{} Command not found!\n",command);
                      }
                  });
    if (!commandExists)
    {
        fmt::print(fmt::fg(fmt::color::medium_spring_green), "Try leaf help\n");
    }
    return 0;
}

void Commands::registerCommands(std::string&&                   command,
                                std::string&&                   description,
                                const std::function<int(void)>& callable)
{
    _commands[command] = {description, callable};
};

const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&
Commands::getCommands() const
{
    return _commands;
}

const std::vector<std::string>& Commands::getArgs() const
{
    return _args;
}
} // namespace Leaf
