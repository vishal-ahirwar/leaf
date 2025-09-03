#include <gtest/gtest.h>
#include "src/main.cpp"

TEST(ReplaceStringTest, BasicReplacement) {
    std::string str = "Hello, world!";
    replaceString(str, "world", "CMake");
    ASSERT_EQ(str, "Hello, CMake!");
}

TEST(ReplaceStringTest, MultipleReplacements) {
    std::string str = "Hello, world! Hello, universe!";
    replaceString(str, "Hello", "Hi");
    ASSERT_EQ(str, "Hi, world! Hi, universe!");
}

TEST(ReplaceStringTest, NoReplacement) {
    std::string str = "Hello, world!";
    replaceString(str, "CMake", "Hi");
    ASSERT_EQ(str, "Hello, world!");
}

TEST(ReplaceStringTest, EmptyString) {
    std::string str = "";
    replaceString(str, "Hello", "Hi");
    ASSERT_EQ(str, "");
}

TEST(ReplaceStringTest, ReplaceWithEmpty) {
    std::string str = "Hello, world!";
    replaceString(str, "world", "");
    ASSERT_EQ(str, "Hello, !");
}

TEST(RunExternalProcessTest, Ls) {
    std::vector<std::string> command = {"ls", "-l"};
    int exit_code = runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}

TEST(RunExternalProcessTest, Echo) {
    std::vector<std::string> command = {"echo", "Hello, world!"};
    int exit_code = runExternalProcess(command);
    ASSERT_EQ(exit_code, 0);
}

TEST(RunExternalProcessTest, InvalidCommand) {
    std::vector<std::string> command = {"invalid_command"};
    int exit_code = runExternalProcess(command);
    ASSERT_NE(exit_code, 0);
}
