
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <leafconfig.h>

#include "commands.h"
#include "leafcommands.h"
#include "utils.h"

int main(int argc, char** argv)
{

    auto args = betterArgs(argc, argv);  
//    args.push_back("create");
    LeafCommands commands(std::move(args));

    return commands.exec();
}
