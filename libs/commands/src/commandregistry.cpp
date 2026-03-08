//
// Created by itsvi on 13-02-2026.
//

#include "commandregistry.h"

#include <cargs/cargs.hpp>
#include <fmt/color.h>

namespace Leaf
{
int commandregistry::exec()
{
    cargs::Parser parser;
    parser.addAlias('r', "release").addAlias('a', "app").addAlias('t', "target");
    const auto parsed = parser.parse(_args);

    _parsedCommand = parsed.command;
    _positionals   = parsed.positionals;
    _options       = parsed.options;

    if (_parsedCommand.empty())
    {
        _default();
        return 1;
    }

    if (const auto it = _commands.find(_parsedCommand); it != _commands.end())
    {
        return it->second.second();
    }

    fmt::print(fmt::fg(fmt::color::red), "{} Command not found!\n", _parsedCommand);
    fmt::print(fmt::fg(fmt::color::medium_spring_green), "Try leaf help\n");
    return 1;
}

void commandregistry::registerCommands(std::string&&                   command,
                                       std::string&&                   description,
                                       const std::function<int(void)>& callable)
{
    _commands[command] = {description, callable};
};

const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&
commandregistry::getCommands() const
{
    return _commands;
}

const std::string& commandregistry::getCommand() const
{
    return _parsedCommand;
}

const std::vector<std::string>& commandregistry::getPositionals() const
{
    return _positionals;
}

bool commandregistry::hasOption(const std::string& option) const
{
    return _options.find(option) != _options.end();
}

std::optional<std::string> commandregistry::getOptionValue(const std::string& option) const
{
    if (const auto it = _options.find(option); it != _options.end() && !it->second.empty())
    {
        return it->second.front();
    }
    return std::nullopt;
}

const std::vector<std::string>& commandregistry::getArgs() const
{
    return _args;
}
} // namespace Leaf
