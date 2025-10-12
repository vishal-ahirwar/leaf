#pragma once

#include <string>

namespace Leaf
{
void download(const std::string& url, const std::string& filePath);

void downloadGithubDirectory(const std::string& owner,
                             const std::string& repo,
                             const std::string& directory,
                             const std::string& localPath);

} // namespace Leaf
