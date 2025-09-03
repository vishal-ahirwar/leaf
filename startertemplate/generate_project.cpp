#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <map>
#include <algorithm>

namespace fs = std::filesystem;

// Function to replace all occurrences of a substring in a string
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <NewProjectName> <AppName> <LibName>" << std::endl;
        std::cerr << "Example: " << argv[0] << " MyCoolProject Game Core" << std::endl;
        return 1;
    }

    std::string projectName = argv[1];
    std::string appName = argv[2];
    std::string libName = argv[3];

    fs::path sourceDir = ".";
    fs::path destDir = fs::path("..") / projectName;

    std::map<std::string, std::string> replacements = {
        {"%PROJECT_NAME%", projectName},
        {"%APPNAME%", appName},
        {"%LIBNAME%", libName}
    };

    try {
        // 1. Copy the entire template directory to a new location
        std::cout << "Copying template to " << destDir << "..." << std::endl;
        fs::copy(sourceDir, destDir, fs::copy_options::recursive);

        // 2. Collect all paths before renaming to avoid iterator invalidation
        std::vector<fs::path> paths;
        for (const auto& entry : fs::recursive_directory_iterator(destDir)) {
            paths.push_back(entry.path());
        }

        // 3. Sort paths in reverse order (deepest first) to handle renaming properly
        // This ensures child directories/files are renamed before their parents.
        std::sort(paths.rbegin(), paths.rend());

        // 4. Rename files and directories
        std::cout << "Renaming files and directories..." << std::endl;
        for (const auto& path : paths) {
            std::string pathStr = path.filename().string();
            std::string newPathStr = pathStr;

            for (const auto& pair : replacements) {
                replaceAll(newPathStr, pair.first, pair.second);
            }

            if (pathStr != newPathStr) {
                fs::path newPath = path.parent_path() / newPathStr;
                fs::rename(path, newPath);
                std::cout << "  " << path.string() << " -> " << newPath.string() << std::endl;
            }
        }

        // 5. Replace content within files
        std::cout << "Replacing content in files..." << std::endl;
        for (const auto& entry : fs::recursive_directory_iterator(destDir)) {
            if (entry.is_regular_file()) {
                fs::path filepath = entry.path();
                
                // Read file content
                std::ifstream inFile(filepath);
                if (!inFile) {
                    std::cerr << "Could not open file for reading: " << filepath << std::endl;
                    continue;
                }
                std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
                inFile.close();

                std::string newContent = content;
                for (const auto& pair : replacements) {
                    replaceAll(newContent, pair.first, pair.second);
                }

                // Write modified content back if changed
                if (content != newContent) {
                    std::ofstream outFile(filepath);
                     if (!outFile) {
                        std::cerr << "Could not open file for writing: " << filepath << std::endl;
                        continue;
                    }
                    outFile << newContent;
                    outFile.close();
                    std::cout << "  Updated content in: " << filepath.string() << std::endl;
                }
            }
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nProject '" << projectName << "' generated successfully in " << destDir << std::endl;

    return 0;
}
