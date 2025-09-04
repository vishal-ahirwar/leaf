#include "commands.h"

#include <easyproc.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <utils.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

int install()
{
    std::vector<std::string> conanInstallArgs{"conan",
                                              "install",
                                              ".",
                                              "-of",
                                              ".install",
                                              "-c",
                                              "tools.cmake.cmaketoolchain:user_presets=''"};
    runExternalProcess(conanInstallArgs);
    return 0;
};

int create()
{

    namespace fs = std::filesystem;
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your project? ");
    std::string project_name{};
    std::getline(std::cin, project_name);
    std::map<std::string, std::string> replacements{{"%APPNAME%",project_name},{"%LIBNAME%",project_name},{"startertemplate",project_name}};
    if (std::getenv("HOME"))
    {
        fmt::println("{}",fs::current_path().string());
        auto starter_template=fs::path(std::getenv("HOME"))/".leaf"/"startertemplate";
        starter_template=fs::exists(starter_template)?starter_template:fs::path("startertemplate");
        if (!fs::exists(starter_template))
        {
            fmt::println("Could not find or download start template code!");
            return 1;
        };

        for (const auto& f : fs::recursive_directory_iterator(starter_template))
        {
            std::string filename = f.path().string();
            std::ranges::for_each(replacements, [&filename](const auto& replacement) {replaceString(filename,replacement.first,replacement.second);});
            if (f.is_directory())
            {
                if (fs::create_directories(filename))
                {
                    fmt::println("Directory created {}", filename);
                };
            }else
            {
                fmt::println("File created {}", filename);
                std::ifstream in(f.path());
                std::string content;
                std::ranges::copy(std::istream_iterator<char>(in),std::istream_iterator<char>(),std::back_inserter(content));
                std::ranges::for_each(replacements,[&content](const auto&replacement){replaceString(content,replacement.first,replacement.second);});
                std::ofstream out(filename,std::ios::trunc);
                out.write(content.c_str(),content.size());
                out.close();
            }

        }
    }

    return 0;
};

int publish()
{
    runExternalProcess({"conan", "create", "."});
    return 0;
};

int upload()
{
    return 0;
};

int runTests()
{
    runExternalProcess({"ctest -B build"});
    return 0;
};

int format()
{

    return 0;
};

int clean()
{
    runExternalProcess({"cmake", "-B", "build", "--fresh"});
    return 0;
};

int debug()
{

    return 0;
};

int addPackage()
{
    return 0;
};

int removePackage()
{
    return 0;
};

int addApp()
{
    return 0;
};

int addLib()
{
    return 0;
};

int doctor()
{
    return 0;
};

void betterArgs(std::vector<std::string>& args, size_t argc, char** argv)
{
    for (size_t i = 0; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
}

int build()
{
    runExternalProcess({"cmake", "--build", "build"});
    return 0;
}

int compile()
{
    return 0;
}

int run()
{
    return 0;
}

int init()
{
    return 0;
}
