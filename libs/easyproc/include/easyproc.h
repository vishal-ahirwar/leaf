#pragma once
#include <vector>
#include<string>
namespace Leaf
{


class ProcessHandler{
    inline static std::string log{};
    public:
    static int runExternalProcess(const std::vector<std::string>& args,bool captureStdOutStdErr=true,bool showLog=false);
    static const std::string&getLog();
};

}