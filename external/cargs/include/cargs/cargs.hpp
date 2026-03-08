#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cargs
{

class Parser
{
    std::unordered_map<std::string, std::string> short_to_long_{};

  public:
    Parser& addAlias(char short_opt, std::string long_opt)
    {
        short_to_long_[std::string(1, short_opt)] = std::move(long_opt);
        return *this;
    }

    struct Result
    {
        std::string                                             command{};
        std::vector<std::string>                                positionals{};
        std::unordered_map<std::string, std::vector<std::string>> options{};
    };

    Result parse(const std::vector<std::string>& args) const
    {
        Result out{};
        if (args.size() < 2)
        {
            return out;
        }

        out.command = args[1];
        for (size_t i = 2; i < args.size(); ++i)
        {
            const auto& token = args[i];
            if (token.rfind("--", 0) == 0 && token.size() > 2)
            {
                std::string key = token.substr(2);
                if ((i + 1) < args.size() && args[i + 1].rfind("-", 0) != 0)
                {
                    out.options[key].push_back(args[i + 1]);
                    ++i;
                }
                else
                {
                    out.options[key].push_back("true");
                }
                continue;
            }

            if (token.rfind("-", 0) == 0 && token.size() > 1)
            {
                if (token.size() == 2)
                {
                    std::string key = token.substr(1);
                    if (const auto it = short_to_long_.find(key); it != short_to_long_.end())
                    {
                        key = it->second;
                    }
                    if ((i + 1) < args.size() && args[i + 1].rfind("-", 0) != 0)
                    {
                        out.options[key].push_back(args[i + 1]);
                        ++i;
                    }
                    else
                    {
                        out.options[key].push_back("true");
                    }
                }
                else
                {
                    for (size_t k = 1; k < token.size(); ++k)
                    {
                        std::string key(1, token[k]);
                        if (const auto it = short_to_long_.find(key); it != short_to_long_.end())
                        {
                            key = it->second;
                        }
                        out.options[key].push_back("true");
                    }
                }
                continue;
            }

            out.positionals.push_back(token);
        }
        return out;
    }
};

} // namespace cargs

