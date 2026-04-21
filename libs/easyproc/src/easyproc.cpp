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

struct CustomSink {
    std::string& out;
    bool showLog;
    std::error_code operator()(reproc::stream stream, const uint8_t *buffer, size_t size) {
        if (showLog) {
            if (stream == reproc::stream::err) {
                std::cerr.write(reinterpret_cast<const char*>(buffer), size);
                std::cerr.flush();
            } else {
                std::cout.write(reinterpret_cast<const char*>(buffer), size);
                std::cout.flush();
            }
        }
        out.append(reinterpret_cast<const char*>(buffer), size);
        return {};
    }
};

int ProcessHandler::runExternalProcess(const std::vector<std::string>& args,
                                       bool                            captureStdOutStdErr,
                                       bool                            showLog)
{
    reproc::process process;
    reproc::options options;
    
    options.redirect.parent   = false;
    options.redirect.err.type = reproc::redirect::pipe;
    options.redirect.out.type = reproc::redirect::pipe;
    options.redirect.in.type  = showLog ? reproc::redirect::parent : reproc::redirect::pipe;
    
    auto ec = process.start(args, options);
    if (ec == std::errc::no_such_file_or_directory)
    {
        std::cerr << "Program not found. Make sure it's available from the PATH.\n";
        return ec.value();
    }
    else if (ec)
    {
        return fail(ec);
    }

    // Capture into a local string first, then move under lock
    std::string captured_output;
    if (captureStdOutStdErr)
    {
        CustomSink sink{captured_output, showLog};
        ec = reproc::drain(process, sink, sink);
        if (ec)
        {
            return fail(ec);
        }
    }
    else if (showLog)
    {
        // If capture is false but showLog is true, we still want to stream it to console.
        CustomSink sink{captured_output, showLog};
        ec = reproc::drain(process, sink, sink);
        if (ec)
            return fail(ec);
        // We will just not append to _log lock. 
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
    return _log; 
}

} // namespace EasyProc
