#include <utils.h>
#include<commands.h>
int main(int argc, char** argv)
{
    Leaf::CLI commands(Utils::betterArgs(argc, argv));
    return commands.exec();
}
