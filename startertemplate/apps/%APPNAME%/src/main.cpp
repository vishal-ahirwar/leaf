#include <iostream>
#include "%LIBNAME%.h"
#include"%APPNAME%config.h"
int main() {
    std::cout << get_greet(std::string(Project::NAME)) << std::endl;
    return 0;
}
