//
// Created by itsvi on 10/22/2025.
//

#ifndef LEAF_LEAFCOMMANDS_H
#define LEAF_LEAFCOMMANDS_H
#include <commandregistry.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Leaf
{

class CLI
{

    std::unique_ptr<commandregistry> _commands{};
    std::vector<std::string>         _args{};

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
