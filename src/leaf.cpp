#include <commands.h>
#include <utils.h>

int main(int argc, char** argv)
{
    Leaf::LeafCommands commands(Leaf::betterArgs(argc, argv));
    return commands.exec();
}
