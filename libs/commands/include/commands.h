#pragma once
#include <functional>
#include <map>
#include <ranges>
#include <string>
#include <vector>

class Commands
{

    std::unordered_map<std::string, std::pair<std::string, std::function<int()>>> _commands{};

    std::vector<std::string> _args{};

    std::function<void(void)>_default{};

  public:
    Commands() = delete;
    Commands(std::vector<std::string>&& args,std::function<void(void)>default_func) : _args(args),_default(default_func) {};
    const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&getCommands()const;
    void registerCommands(std::string&&command,std::string&&description,std::function<int(void)>callable);
    const  std::vector<std::string>&getArgs()const;
    int exec();
};