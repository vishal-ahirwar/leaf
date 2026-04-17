#pragma once
#include <string>
#include <vector>
namespace EasyProc
{

class ProcessHandler
{
    inline static std::string _log{};

  public:
    static int runExternalProcess(const std::vector<std::string>& args,
                                  bool                            captureStdOutStdErr = true,
                                  bool                            showLog = false);
    static std::string getLog();
};

} // namespace EasyProc
