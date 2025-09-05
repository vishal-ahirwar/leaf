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
                                              "-s",
                                              "build_type=Release",
                                              "-s",
                                              "&:build_type=Debug",
                                              "-c",
                                              "tools.cmake.cmaketoolchain:user_presets=",
                                              "-o",
                                              "&:build_app=True"};
    runExternalProcess(conanInstallArgs);
    return 0;
};

int create()
{
    namespace fs = std::filesystem;

    std::string lib_name;
    std::string project_name{};

    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green),
               "What would you like to name your project? ");

    std::getline(std::cin, project_name);
    if (project_name.find(" ") != std::string::npos || project_name.empty())
    {
        fmt::println("Project name can't be empty and can't have whitespaces in their name!");
        return 0;
    }
    fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "lib name? ");

    std::getline(std::cin, lib_name);
    if (project_name == lib_name)
    {
        fmt::println("You can't have two targets with same name!");
        return 0;
    };
    lib_name = lib_name.empty() ? project_name + "lib" : lib_name;

    std::map<std::string, std::string> replacements{
        {"%APPNAME%", project_name}, {"%LIBNAME%", lib_name}, {"startertemplate", project_name}};

    if (std::getenv("HOME"))
    {
        fmt::println("{}", std::getenv("HOME"));
        auto starter_template = fs::path(std::getenv("HOME")) / ".leaf" / "startertemplate";
        if (!fs::exists(starter_template))
        {
            starter_template = fs::path("startertemplate");
        }
        if (!fs::exists(starter_template))
        {
            fmt::println("Could not find or download start template code!");
            return 1;
        };

        for (const auto& f : fs::recursive_directory_iterator(starter_template))
        {
            std::string filename = f.path().string();
            std::ranges::for_each(
                replacements,
                [&filename](const auto& replacement)
                { replaceString(filename, replacement.first, replacement.second); });
            auto is_directory = f.is_directory();

            if (is_directory)
            {
                if (fs::create_directories(filename))
                {
                    fmt::println("Directory created {}", filename);
                };
            }
            else
            {
                fmt::println("File created {}", filename);
                std::ifstream in(f.path());
                std::string   content;
                in >> std::noskipws;
                std::ranges::copy(std::istream_iterator<char>(in),
                                  std::istream_iterator<char>(),
                                  std::back_inserter(content));
                std::ranges::for_each(
                    replacements,
                    [&content](const auto& replacement)
                    { replaceString(content, replacement.first, replacement.second); });
                std::ofstream out(filename);
                out.write(content.c_str(), content.size());
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
    runExternalProcess({"cmake", "--build", "--preset","debug", "--fresh"});
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
    namespace fs = std::filesystem;
    if (!fs::exists(".install"))
    {
        install();
    };
    if (!fs::exists(".build"))
    {
        runExternalProcess({"cmake", "--preset", "debug"});
    }

    runExternalProcess({"cmake", "--build", "--preset", "debug"});
    return 0;
}

int compile()
{
    return 0;
}

int run()
{
#ifdef _WIN322
    std::string extention = ".exe";
#else
    std::string extention = "";
#endif
    std::system(fmt::format("./.build/Debug/apps/{}/{}{}", "test", "test", extention).c_str());
    return 0;
}

int init()
{
    return 0;
}
