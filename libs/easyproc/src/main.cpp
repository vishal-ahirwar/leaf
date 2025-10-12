#include <fmt/color.h>
#include <fmt/core.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <reproc++/reproc.hpp>
#include <reproc++/run.hpp>
#include <string>
#include <vector>

#include "../include/easyproc.h"

namespace Leaf
{

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
        fmt::print(fmt::emphasis::bold | fmt::fg(fmt::color::light_green), "Executing ");
        std::ranges::copy(args, std::ostream_iterator<std::string>{std::cout, " "});
        std::cout << std::endl;
    }

    reproc::process process;
    reproc::options options;
    options.redirect.parent   = showLog;
    options.redirect.err.type = reproc::redirect::pipe;
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

    // `reproc::drain` reads from the stdout and stderr streams of `process` until
    // both are closed or an error occurs. Providing it with a string sink for a
    // specific stream makes it store all output of that stream in the string
    // passed to the string sink. Passing the same sink to both the `out` and
    // `err` arguments of `reproc::drain` causes the stdout and stderr output to
    // get stored in the same string.
    if (captureStdOutStdErr)
    {
        log.clear();
        reproc::sink::string sink(log);
        // By default, reproc only redirects stdout to a pipe and not stderr so we
        // pass `reproc::sink::null` as the sink for stderr here. We could also pass
        // `sink` but it wouldn't receive any data from stderr.
        ec = reproc::drain(process, sink, sink);
        if (ec)
        {
            return fail(ec);
        }
    }

    // It's easy to define your own sinks as well. Take a look at `drain.hpp` in
    // the repository to see how `sink::string` and other sinks are implemented.
    // The documentation of `reproc::drain` also provides more information on the
    // requirements a sink should fulfill.

    // By default, The `process` destructor waits indefinitely for the child
    // process to exit to ensure proper cleanup. See the forward example for
    // information on how this can be configured. However, when relying on the
    // `process` destructor, we cannot check the exit status of the process so it
    // usually makes sense to explicitly wait for the process to exit and check
    // its exit status.

    int status           = 0;
    std::tie(status, ec) = process.wait(reproc::infinite);
    if (ec)
    {
        return fail(ec);
    }
    if (captureStdOutStdErr)
    {
        std::ofstream out("build.log");
        out << log;
        out.close();
    }

    return status;
}

const std::string& ProcessHandler::getLog()
{
    return ProcessHandler::log;
};
} // namespace Leaf