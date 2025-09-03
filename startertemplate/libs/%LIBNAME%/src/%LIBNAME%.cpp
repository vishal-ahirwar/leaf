#include "%LIBNAME%.h"
#include<format>
std::string get_greet(const std::string& name) {
    return std::format("Hello, {}!", name);
}
