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
namespace Leaf
{

class CommandRegistry
{

    std::unordered_map<std::string, std::pair<std::string, std::function<int()>>> _commands{};

    std::vector<std::string> _args{};

    std::function<void(void)> _default{};

  public:
    CommandRegistry() = delete;
    CommandRegistry(std::vector<std::string>&& args, std::function<void(void)> default_func)
        : _args(args), _default(default_func) {};
    const std::unordered_map<std::string, std::pair<std::string, std::function<int()>>>&
                                    getCommands() const;
    void                            registerCommands(std::string&&                   command,
                                                     std::string&&                   description,
                                                     const std::function<int(void)>& callable);
    const std::vector<std::string>& getArgs() const;
    int                             exec();
};

class CLI
{

    std::unique_ptr<CommandRegistry> _commands{};
    std::vector<std::string>  _args{};

public:
    CLI() = delete;
    CLI(std::vector<std::string>&& args);
    int exec();

protected:

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
} // namespace Leaf



#endif // LEAF_LEAFCOMMANDS_H
