#include <commands.h>
#include <utils.h>

int main(int argc, char** argv)
{
    auto args = betterArgs(argc, argv);
    LeafCommands commands(std::move(args));
    return commands.exec();
}
