#include <Commands.h>
#include <utils.h>

int main(int argc, char** argv)
{
    LeafCommands::LeafCommands commands(Utils::betterArgs(argc, argv));
    return commands.exec();
}
