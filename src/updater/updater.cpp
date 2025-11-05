#include <iostream>

#include "leafconfig.h"
int main(int argc, char** argv)
{
    std::cout << "Updater v" << Project::VERSION_STRING << std::endl;
    return 0;
}
