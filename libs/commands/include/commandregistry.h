//
// Created by itsvi on 13-02-2026.
//

#ifndef LEAF_COMMANDREGISTRY_H
#define LEAF_COMMANDREGISTRY_H
#include <functional>
#include <unordered_map>
#include <vector>
#include<string>

namespace Leaf
{

class commandregistry
{

    std::unordered_map<std::string,
                       std::pair<std::string, std::function<int()>>>
        _commands{};

    std::vector<std::string> _args{};

    std::function<void(void)> _default{};

public:
    commandregistry() = delete;
    commandregistry(std::vector<std::string>&& args, std::function<void(void)> default_func)
        : _args(args), _default(default_func) {};
    const std::unordered_map<std::string,
                             std::pair<std::string, std::function<int()>>>&
                                    getCommands() const;
    void                            registerCommands(std::string&&              command,
                                                     std::string&&              description,
                                                     const std::function<int(void)>& callable);
    const std::vector<std::string>& getArgs() const;
    int                             exec();
};

} // namespace Leaf

#endif // LEAF_COMMANDREGISTRY_H
