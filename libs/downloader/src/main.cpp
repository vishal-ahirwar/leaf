#include <cpr/cpr.h>
#include <cpr/ssl_options.h>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "../include/downloder.h"

std::string urlEncode(const std::string& value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value)
    {
        // Keep alphanumeric and other unreserved characters
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }
        // Any other characters are escaped
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

void download(const std::string& url, const std::string& outputFilePath)
{
#ifdef _WIN32
    std::string name = outputFilePath.substr(outputFilePath.find_last_of("\\") + 1);
#else
    std::string name = outputFilePath.substr(outputFilePath.find_last_of("/") + 1);
#endif

    cpr::Response response =
        cpr::Get(cpr::Url{url},
                 cpr::SslOptions{},
                 cpr::ReserveSize{1024 * 1024 * 8},
                 cpr::ProgressCallback(
                     [&](cpr::cpr_off_t download_total,
                         cpr::cpr_off_t download_now,
                         cpr::cpr_off_t upload_total,
                         cpr::cpr_off_t upload_now,
                         intptr_t       user_data) -> bool
                     {
                         fmt::print("\r{}",
                                    fmt::format("Downloading \033[32m{}\033[0m : {:.2f}%",
                                                name.c_str(),
                                                ((double) download_now / download_total) * 100.0),
                                    "\r");
                         return true;
                     }));

    if (response.status_code == 200)
    {

        std::ofstream outputFile(outputFilePath, std::ios::binary);
        if (!outputFile.is_open())
        {
            fmt::println(
                "\n{}",
                fmt::format("failed to save  \033[32m{}\033[0m", outputFilePath));  
            outputFile.close();
            return;
        }
        outputFile.write(response.text.c_str(), response.text.size());
        outputFile.close();
        fmt::println("\n{}",
                     fmt::format("file downloaded and saved as \033[32m{}\033[0m", outputFilePath));
    }
    else
    {
        fmt::println("\n{}",
                     fmt::format("{}. Status code:\033[32m{}\033[0m",response.status_line,
                                 response.status_code));
    }
}

void downloadGithubDirectory(const std::string& owner,
                             const std::string& repo,
                             const std::string& directory,
                             const std::string& localPath)
{
    // 1. Construct the GitHub API URL
    std::string encodedRepoPath = urlEncode(directory);
    std::string apiUrl =
        fmt::format("https://api.github.com/repos/{}/{}/contents/{}", owner, repo, encodedRepoPath);
    fmt::print("-> Fetching contents of directory: {}\n", directory.empty() ? "/" : directory);

    // 2. Make the API request
    cpr::Response r = cpr::Get(
        cpr::Url{apiUrl}, cpr::SslOptions{}, cpr::Header{{"User-Agent", "C++ Downloader"}});

    if (r.status_code != 200)
    {
        fmt::print(stderr,
                   fg(fmt::color::red),
                   "Failed to fetch directory contents. Status: {}\n",
                   r.status_code);
        fmt::print(stderr, "  Response: {}\n", r.text);
        return;
    }

    // 3. Parse the JSON response
    auto json_response = nlohmann::json::parse(r.text);

    // 4. Iterate through each item in the directory
    for (const auto& item : json_response)
    {
        std::string itemName     = item["name"];
        std::string itemType     = item["type"];
        std::string itemRepoPath = item["path"];

        // Construct the full local path for the item
        std::filesystem::path localItemPath = std::filesystem::path(localPath) / itemName;

        if (itemType == "file")
        {
            // It's a file, download it
            std::string downloadUrl = item["download_url"];
            fmt::print("  - Found file: {}\n", itemName);
            download(downloadUrl, localItemPath.string());
        }
        else if (itemType == "dir")
        {
            // It's a directory, create it locally and recurse
            fmt::print("  - Found directory: {}. Recursing...\n", itemName);

            // Create the local directory
            std::filesystem::create_directories(localItemPath);

            // Recursively call this function for the new directory
            downloadGithubDirectory(owner, repo, itemRepoPath, localItemPath.string());
        }
    }
}