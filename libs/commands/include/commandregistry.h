//
// Created by itsvi on 13-02-2026.
//

#ifndef LEAF_COMMANDREGISTRY_H
#define LEAF_COMMANDREGISTRY_H
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Leaf
{

class commandregistry
{

    std::unordered_map<std::string, std::pair<std::string, std::function<int()>>> _commands{};

    std::vector<std::string>                                  _args{};
    std::string                                               _parsedCommand{};
    std::vector<std::string>                                  _positionals{};
    std::unordered_map<std::string, std::vector<std::string>> _options{};

    std::function<void(void)> _default{};

  public:
    commandregistry() = delete;
    commandregistry(const std::vector<std::string>& args, std::function<void(void)> default_func)
        : _args(args), _default(default_func) {};
    const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&
                                    getCommands() const;
    void                            registerCommands(std::string&&                   command,
                                                     std::string&&                   description,
                                                     const std::function<int(void)>& callable);
    const std::string&              getCommand() const;
    const std::vector<std::string>& getPositionals() const;
    bool                            hasOption(const std::string& option) const;
    std::optional<std::string>      getOptionValue(const std::string& option) const;
    const std::vector<std::string>& getArgs() const;
    int                             exec();
};

} // namespace Leaf

#endif // LEAF_COMMANDREGISTRY_H
