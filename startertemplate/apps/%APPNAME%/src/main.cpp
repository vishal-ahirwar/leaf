#include <%APPNAME%config.h>
#include <iostream>
#include <%LIBNAME%.h>
int main() {
    hello(std::string(Project::COMPANY));
    return 0;
}
