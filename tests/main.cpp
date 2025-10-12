#include <gtest/gtest.h>

#include <string>

#include "easyproc.h"
#include "utils.h"
#include <logger.h>
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
    Leaf::replaceString(str, from, to);
    ASSERT_EQ(str, "Hello, CMake!");
}

TEST(ReplaceStringTest, MultipleReplacements)
{
    std::string str  = "Hello, world! Hello, universe!";
    std::string from = "Hello";
    std::string to   = "Hi";
    Leaf::replaceString(str, from, to);
    ASSERT_EQ(str, "Hi, world! Hi, universe!");
}

TEST(ReplaceStringTest, NoReplacement)
{
    std::string str  = "Hello, world!";
    std::string from = "CMake";
    std::string to   = "Hi";
    Leaf::replaceString(str, from, to);
    ASSERT_EQ(str, "Hello, world!");
}

TEST(ReplaceStringTest, EmptyString)
{
    std::string str  = "";
    auto        from = "Hello"s;
    auto        to   = "Hi"s;
    Leaf::replaceString(str, from, to);
    ASSERT_EQ(str, "");
}

TEST(ReplaceStringTest, ReplaceWithEmpty)
{
    std::string str  = "Hello, world!";
    auto        from = "world"s;
    auto        to   = ""s;
    Leaf::replaceString(str, from, to);
    ASSERT_EQ(str, "Hello, !");
}

TEST(RunExternalProcessTest, Ls)
{
    std::vector<std::string> command   = {"ls", "-l"};
    int                      exit_code = Leaf::ProcessHandler:: runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}

TEST(RunExternalProcessTest, Echo)
{
    std::vector<std::string> command   = {"echo", "Hello, world!"};
    int                      exit_code = Leaf::ProcessHandler:: runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}
TEST(RunExternalProcessTest, Clang)
{
    std::vector<std::string> command   = {"clang", "--version"};
    int                      exit_code = Leaf::ProcessHandler::runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}
TEST(RunExternalProcessTest, InvalidCommand)
{
    std::vector<std::string> command   = {"invalid_command"};
    int                      exit_code = Leaf::ProcessHandler:: runExternalProcess(command);
    ASSERT_NE(exit_code, 0);
}
