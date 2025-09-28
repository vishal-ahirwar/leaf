
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <leafconfig.h>

#include "../../libs/commands/include/commands.h"
#include "../../libs/utils/include/utils.h"
#include "leafcommands.h"

int main(int argc, char** argv)
{

    auto args = betterArgs(argc, argv);
    // args.push_back("help");
    LeafCommands commands(std::move(args));

    return commands.exec();
}
