//
// Created by itsvi on 10/22/2025.
//

#ifndef LEAF_LEAFCOMMANDS_H
#define LEAF_LEAFCOMMANDS_H
#include <commandregistry.h>

#include <functional>
#include <map>
#include <memory>
#include <optional>
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
    bool                       isReleaseMode() const;
    bool                       isVerboseMode() const;
    std::optional<std::string> getAppOption() const;
    std::optional<std::string> getTargetOption() const;
    std::string                detectDefaultAppName() const;

    int build();
    int compile();
    int run();
    int clean();

    int create();
    int init();
    int addApp();
    int addLib();

    int install();
    int addPackage();
    int removePackage();
    int publish();
    int upload();

    int doctor();
    int format();
    int setupToolChain();
    int generateProfile();
    int runTests();
    int generateDocs();
    int startLeafUpdater();

    int search();
    int info();
    int depTree();

    int server();

    int help();
    int version() const;
};
} // namespace Leaf

#endif // LEAF_LEAFCOMMANDS_H
