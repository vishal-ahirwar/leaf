#include <commands.h>
#include <utils.h>

int main(int argc, char** argv)
{
    LeafCommands commands(betterArgs(argc, argv));
    return commands.exec();
}
