//
// Created by itsvi on 10/22/2025.
//

#ifndef LEAF_LEAFCOMMANDS_H
#define LEAF_LEAFCOMMANDS_H
#include <memory>
#include <string>
#include <vector>
#include<commands.h>

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
