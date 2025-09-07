
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <leafconfig.h>

#include "commands.h"
#include "leafcommands.h"
#include "utils.h"

int main(int argc, char** argv)
{
    LeafCommands commands(betterArgs(argc,argv));
    return commands.exec();
}
