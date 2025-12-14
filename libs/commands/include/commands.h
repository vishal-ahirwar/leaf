//
// Created by itsvi on 10/22/2025.
//

#ifndef LEAF_LEAFCOMMANDS_H
#define LEAF_LEAFCOMMANDS_H
#include <memory>
#include <string>
#include <vector>
#include<unordered_map>
#include <functional>
#include<map>
namespace Commands
{

class Commands
{

    std::unordered_map<std::string, std::pair<std::string, std::function<int()>>> _commands{};

    std::vector<std::string> _args{};

    std::function<void(void)> _default{};

  public:
    Commands() = delete;
    Commands(std::vector<std::string>&& args, std::function<void(void)> default_func)
        : _args(args), _default(default_func) {};
    const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&
                                    getCommands() const;
    void                            registerCommands(std::string&&                   command,
                                                     std::string&&                   description,
                                                     const std::function<int(void)>& callable);
    const std::vector<std::string>& getArgs() const;
    int                             exec();
};
}; // namespace Commands

namespace LeafCommands
{

class LeafCommands
{

    std::unique_ptr<Commands::Commands> _commands{};
    std::vector<std::string>  _args{};

public:
    LeafCommands() = delete;
    LeafCommands(std::vector<std::string>&& args);
    int exec();

private:

    int generateProfile();

    int install();

    int create();

    int publish();

    int upload();

    int runTests();

    int format();

    int clean();

    int release();

    int addPackage();

    int removePackage();

    int addApp();

    int addLib();

    int doctor();

    int build();

    int compile();

    int run();

    int init();

    int help();

    int generateDocs();

    int version() const;

    int setupToolChain();

    int startLeafUpdater();
};
}


#endif // LEAF_LEAFCOMMANDS_H
