#include"commands.h"
#include<vector>
#include<string>
#include<utils.h>

int install()
{
  std::vector<std::string> conanInstallArgs{"conan", "install", ".", "-of", ".install", "-c", "tools.cmake.cmaketoolchain:user_presets=''"};
  runExternalProcess(conanInstallArgs);
  return 0;
};

int create()
{
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
  runExternalProcess({"cmake", "-B", "Build", "--fresh"});
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

void betterArgs(std::vector<std::string> &args, size_t argc, char **argv)
{
  for (size_t i = 0; i < argc; i++)
  {
    args.push_back(argv[i]);
  }
}

int help()
{
    return 0;
}
