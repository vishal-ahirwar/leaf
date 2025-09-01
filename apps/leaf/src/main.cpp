
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <format>
#include <leafconfig.h>
#include <fmt/core.h>
#include <reproc++/run.hpp>
#include <fmt/color.h>
#include <iterator>

int install(const std::vector<std::string> &args)
{
  std::vector<std::string> conanInstallArgs{"conan", "install", ".", "-of", ".install", "-c", "tools.cmake.cmaketoolchain:user_presets=''"};
  std::ranges::copy_if(args, std::back_inserter(conanInstallArgs), [&conanInstallArgs](const std::string &arg)
                       { return std::ranges::find(conanInstallArgs, arg) == std::end(conanInstallArgs); });
  std::ranges::copy(conanInstallArgs, std::ostream_iterator<std::string>{std::cout, " "});
  std::cout << std::endl;
  return 0;
};

int create(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int publish(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int upload(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int runTests(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int format(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int clean(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int debug(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int addPackage(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int removePackage(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int addApp(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int addLib(const std::vector<std::string> &args)
{
  std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

int doctor(std::vector<std::string> &result)
{
  std::ranges::copy(result, std::ostream_iterator<std::string>{std::cout, " "});
  return 0;
};

void betterArgs(std::vector<std::string> &args, size_t argc, char **argv)
{
  for (size_t i = 0; i < argc; i++)
  {
    args.push_back(argv[i]);
  }
}
int main(int argc, char **argv)
{
  fmt::print("🍃 Leaf ");
  fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "v{} by {}\nA modern, fast, and intuitive project/package manager for C++\n", Project::VERSION_STRING, Project::COMPANY_NAME);

  std::vector<std::string> args{};
  betterArgs(args, argc, argv);

  std::string build_type{"Debug"};
  //install(std::vector<std::string>{"-s", std::format("build_type={}", build_type)});
  return 0;
}