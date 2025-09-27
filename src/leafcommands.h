#pragma once
#include <commands.h>
#include <fmt/color.h>
#include <fmt/core.h>

#include <functional>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

class LeafCommands
{
    std::unique_ptr<Commands> _commands{};
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

    int version() const;
};