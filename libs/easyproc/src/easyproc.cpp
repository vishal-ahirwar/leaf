#include "../include/easyproc.h"

#include <fmt/color.h>
#include <fmt/core.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <mutex>
#include <ranges>
#include <reproc++/reproc.hpp>
#include <reproc++/run.hpp>
#include <string>
#include <vector>

namespace EasyProc
{

static std::mutex s_log_mutex;

static int fail(std::error_code ec)
{
    std::cerr << ec.message();
    return ec.value();
}

int ProcessHandler::runExternalProcess(const std::vector<std::string>& args,
                                       bool                            captureStdOutStdErr,
                                       bool                            showLog)
{

    if (showLog)
    {
        std::string cmd_line;
        for (const auto& arg : args)
        {
            if (arg.find(' ') != std::string::npos)
            {
                cmd_line += fmt::format("\"{}\" ", arg);
            }
            else
            {
                cmd_line += arg + " ";
            }
        }
        return std::system(cmd_line.c_str());
    }
    reproc::process process;
    reproc::options options;
    options.redirect.parent   = showLog;
    options.redirect.err.type = reproc::redirect::pipe;
    options.redirect.in.type  = reproc::redirect::pipe;
    auto ec                   = process.start(args, options);
    if (ec == std::errc::no_such_file_or_directory)
    {
        std::cerr << "Program not found. Make sure it's available from the PATH.";
        return ec.value();
    }
    else if (ec)
    {
        return fail(ec);
    }

    // Capture into a local string first, then move under lock — minimal lock time
    std::string captured_output;
    if (captureStdOutStdErr)
    {
        reproc::sink::string sink(captured_output);
        ec = reproc::drain(process, sink, sink);
        if (ec)
        {
            return fail(ec);
        }
    }

    int status           = 0;
    std::tie(status, ec) = process.wait(reproc::infinite);
    if (ec)
    {
        return fail(ec);
    }

    if (captureStdOutStdErr)
    {
        std::lock_guard<std::mutex> guard(s_log_mutex);
        _log = std::move(captured_output);
    }

    return status;
}

std::string ProcessHandler::getLog()
{
    std::lock_guard<std::mutex> guard(s_log_mutex);
    return _log; // Return by value for thread safety
}

} // namespace EasyProc
