#include <gtest/gtest.h>
#include <logger.h>

#include <string>
#include <filesystem>

#include "../libs/commands/include/commands.h"
#include "easyproc.h"
#include "utils.h"
using namespace std::string_literals;


TEST(Logger,log)
{
    Leaf::Logger::log("Initiating test");
    Leaf::Logger::log("Almost done");
    Leaf::Logger::log("Test completed");
}
TEST(ReplaceStringTest, BasicReplacement)
{
    std::string str  = "Hello, world!";
    std::string from = "world";
    std::string to   = "CMake";
    Utils::replaceString(str, from, to);
    ASSERT_EQ(str, "Hello, CMake!");
}

TEST(ReplaceStringTest, MultipleReplacements)
{
    std::string str  = "Hello, world! Hello, universe!";
    std::string from = "Hello";
    std::string to   = "Hi";
    Utils::replaceString(str, from, to);
    ASSERT_EQ(str, "Hi, world! Hi, universe!");
}

TEST(ReplaceStringTest, NoReplacement)
{
    std::string str  = "Hello, world!";
    std::string from = "CMake";
    std::string to   = "Hi";
    Utils::replaceString(str, from, to);
    ASSERT_EQ(str, "Hello, world!");
}

TEST(ReplaceStringTest, EmptyString)
{
    std::string str  = "";
    auto        from = "Hello"s;
    auto        to   = "Hi"s;
    Utils::replaceString(str, from, to);
    ASSERT_EQ(str, "");
}

TEST(ReplaceStringTest, ReplaceWithEmpty)
{
    std::string str  = "Hello, world!";
    auto        from = "world"s;
    auto        to   = ""s;
    Utils::replaceString(str, from, to);
    ASSERT_EQ(str, "Hello, !");
}

TEST(RunExternalProcessTest, Ls)
{
    std::vector<std::string> command   = {"ls", "-l"};
    int                      exit_code = EasyProc::ProcessHandler:: runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}

TEST(RunExternalProcessTest, Echo)
{
    std::vector<std::string> command   = {"echo", "Hello, world!"};
    int                      exit_code = EasyProc::ProcessHandler:: runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}
TEST(RunExternalProcessTest, Clang)
{
    std::vector<std::string> command   = {"clang", "--version"};
    int                      exit_code = EasyProc::ProcessHandler::runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}
TEST(RunExternalProcessTest, InvalidCommand)
{
    std::vector<std::string> command   = {"invalid_command"};
    int                      exit_code = EasyProc::ProcessHandler:: runExternalProcess(command);
    ASSERT_NE(exit_code, 0);
}


//--------------Profile Gen-----------

TEST(CMakeToConanProfile,Generation)
{
    class HighCommand:public Leaf::CLI
    {
    public:
        HighCommand() : Leaf::CLI(std::vector<std::string>())
        {
        }
        int run(){return generateProfile();};
    };


    HighCommand highCommand;
    ASSERT_EQ(highCommand.run(),0);

    std::string os_profile;
#ifdef _WIN32
    os_profile = "profiles/windows_profile";
#elif defined(__APPLE__)
    os_profile = "profiles/macos_profile";
#else
    os_profile = "profiles/linux_profile";
#endif
    ASSERT_TRUE(std::filesystem::exists(os_profile));
}

TEST(Docs,Generation)
{
    class HighCommand:public Leaf::CLI
    {
    public:
        HighCommand() : Leaf::CLI(std::vector<std::string>())
        {
        }
        int run(){return generateDocs();};
    };


    HighCommand highCommand;
    ASSERT_EQ(highCommand.run(),0);
}