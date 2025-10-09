#include "%LIBNAME%.h"
#include <iostream>
namespace %LIBNAME%{
    void hello(const std::string& name) {
        std::cout << "Hello, "<<name << std::endl;
    }
}