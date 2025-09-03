#include <iostream>
#include "%LIBNAME%.h"
#include"%APPNAME%config.h"
int main() {
    std::cout << get_greet(std::format("{} v{}",Project::NAME,Project::VERSION)) << std::endl;
    return 0;
}
