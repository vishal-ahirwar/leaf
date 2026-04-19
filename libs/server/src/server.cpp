#include "server.h"

#include <WebAssets.h>
#include <cpp-embedlib-httplib.h>
#include <httplib.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace server
{
namespace
{

namespace fs = std::filesystem;
std::mutex g_debug_log_mutex;
fs::path   g_debug_log_path;

void append_debug_log(const std::string& line)
{
    std::lock_guard<std::mutex> lock(g_debug_log_mutex);
    if (g_debug_log_path.empty())
    {
        return;
    }
    std::ofstream log(g_debug_log_path, std::ios::app);
    log << line << '\n';
}

struct User
{
    std::string username;
    std::string password;
};

struct Session
{
    std::string                           username;
    std::string                           token;
    std::chrono::system_clock::time_point expires_at;
};

struct RevisionInfo
{
    std::string revision;
    std::string time;
    fs::path    path;
};

struct RecipeRef
{
    std::string name;
    std::string version;
    std::string user;
    std::string channel;
};

struct ServerConfig
{
    std::string host         = "0.0.0.0";
    int         port         = 9300;
    fs::path    storage_root = ".leafserver-data";
    std::string admin_user   = "admin";
    std::string admin_password;
};

std::string trim(std::string value)
{
    auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::string json_escape(std::string_view value)
{
    std::string out;
    out.reserve(value.size() + 16);
    for (const char ch : value)
    {
        switch (ch)
        {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20U)
            {
                std::ostringstream hex;
                hex << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<int>(static_cast<unsigned char>(ch));
                out += hex.str();
            }
            else
            {
                out += ch;
            }
            break;
        }
    }
    return out;
}

std::string iso8601_now()
{
    const auto now  = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    const auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3) << std::setfill('0')
        << ms.count() << "+0000";
    return out.str();
}

std::string random_token(std::size_t length)
{
    static constexpr char kAlphabet[] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device                         rd;
    std::mt19937                               gen(rd());
    std::uniform_int_distribution<std::size_t> dist(0, sizeof(kAlphabet) - 2);
    std::string                                token;
    token.reserve(length);
    for (std::size_t i = 0; i < length; ++i)
    {
        token.push_back(kAlphabet[dist(gen)]);
    }
    return token;
}

fs::path platform_fs_path(const fs::path& input)
{
#ifdef _WIN32
    fs::path           absolute = fs::absolute(input);
    const std::wstring value    = absolute.native();
    if (value.rfind(L"\\\\?\\", 0) == 0)
    {
        return absolute;
    }
    if (value.rfind(L"\\\\", 0) == 0)
    {
        return fs::path(L"\\\\?\\UNC\\" + value.substr(2));
    }
    return fs::path(L"\\\\?\\" + value);
#else
    return input;
#endif
}

bool is_safe_path_segment(std::string_view value)
{
    return !value.empty() && value.find("..") == std::string_view::npos &&
           value.find('\\') == std::string_view::npos && value.find('/') == std::string_view::npos;
}

std::string base64_decode(std::string_view input)
{
    static constexpr unsigned char kInvalid   = 0xFF;
    static constexpr unsigned char table[256] = {
        kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid,
        kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid,
        kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid,
        kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid,
        kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, 62,       kInvalid,
        kInvalid, kInvalid, 63,       52,       53,       54,       55,       56,       57,
        58,       59,       60,       61,       kInvalid, kInvalid, kInvalid, kInvalid, kInvalid,
        kInvalid, kInvalid, 0,        1,        2,        3,        4,        5,        6,
        7,        8,        9,        10,       11,       12,       13,       14,       15,
        16,       17,       18,       19,       20,       21,       22,       23,       24,
        25,       kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, kInvalid, 26,       27,
        28,       29,       30,       31,       32,       33,       34,       35,       36,
        37,       38,       39,       40,       41,       42,       43,       44,       45,
        46,       47,       48,       49,       50,       51,       kInvalid, kInvalid, kInvalid,
        kInvalid, kInvalid,
    };

    std::string out;
    int         val  = 0;
    int         valb = -8;
    for (const unsigned char c : input)
    {
        if (std::isspace(c) != 0)
        {
            continue;
        }
        if (c == '=')
        {
            break;
        }
        const unsigned char decoded = table[c];
        if (decoded == kInvalid)
        {
            return {};
        }
        val = (val << 6) + decoded;
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(static_cast<char>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::optional<std::pair<std::string, std::string>> parse_basic_header(std::string_view header)
{
    static constexpr std::string_view prefix = "Basic ";
    if (!header.starts_with(prefix))
    {
        return std::nullopt;
    }
    const std::string decoded = base64_decode(header.substr(prefix.size()));
    const auto        split   = decoded.find(':');
    if (split == std::string::npos)
    {
        return std::nullopt;
    }
    return std::make_pair(decoded.substr(0, split), decoded.substr(split + 1));
}

std::optional<std::string> parse_bearer_header(std::string_view header)
{
    static constexpr std::string_view prefix = "Bearer ";
    if (!header.starts_with(prefix))
    {
        return std::nullopt;
    }
    return std::string(header.substr(prefix.size()));
}

class PackageStorage
{
  public:
    explicit PackageStorage(fs::path root) : root_(std::move(root))
    {
        fs::create_directories(root_);
    }

    [[nodiscard]] fs::path root() const
    {
        return root_;
    }
    [[nodiscard]] fs::path config_path() const
    {
        return root_ / "leafserver.conf";
    }

    [[nodiscard]] fs::path recipe_base(const RecipeRef& ref) const
    {
        return root_ / "recipes" / ref.name / ref.version / ref.user / ref.channel;
    }

    [[nodiscard]] fs::path recipe_revision_path(const RecipeRef& ref,
                                                std::string_view revision) const
    {
        return recipe_base(ref) / "revisions" / std::string(revision);
    }

    [[nodiscard]] fs::path recipe_files_path(const RecipeRef& ref, std::string_view revision) const
    {
        return recipe_revision_path(ref, revision) / "files";
    }

    [[nodiscard]] fs::path package_revision_path(const RecipeRef& ref,
                                                 std::string_view recipe_revision,
                                                 std::string_view package_id,
                                                 std::string_view package_revision) const
    {
        return recipe_revision_path(ref, recipe_revision) / "packages" / std::string(package_id) /
               "revisions" / std::string(package_revision);
    }

    [[nodiscard]] fs::path package_files_path(const RecipeRef& ref,
                                              std::string_view recipe_revision,
                                              std::string_view package_id,
                                              std::string_view package_revision) const
    {
        return package_revision_path(ref, recipe_revision, package_id, package_revision) / "files";
    }

    void ensure_layout() const
    {
        fs::create_directories(root_ / "recipes");
    }

    [[nodiscard]] std::vector<RevisionInfo> list_recipe_revisions(const RecipeRef& ref) const
    {
        return list_revisions(recipe_base(ref) / "revisions");
    }

    [[nodiscard]] std::vector<RevisionInfo>
    list_package_revisions(const RecipeRef& ref,
                           std::string_view recipe_revision,
                           std::string_view package_id) const
    {
        return list_revisions(recipe_revision_path(ref, recipe_revision) / "packages" /
                              std::string(package_id) / "revisions");
    }

    [[nodiscard]] std::vector<std::string> list_files(const fs::path& files_dir) const
    {
        std::vector<std::string> files;
        if (!fs::exists(files_dir))
        {
            return files;
        }
        for (const auto& entry : fs::directory_iterator(files_dir))
        {
            if (entry.is_regular_file())
            {
                files.push_back(entry.path().filename().string());
            }
        }
        std::sort(files.begin(), files.end());
        return files;
    }

    [[nodiscard]] std::vector<RecipeRef> list_recipe_refs() const
    {
        std::vector<RecipeRef> refs;
        const fs::path         recipes_root = root_ / "recipes";
        if (!fs::exists(recipes_root))
        {
            return refs;
        }
        for (const auto& name_dir : fs::directory_iterator(recipes_root))
        {
            if (!name_dir.is_directory())
            {
                continue;
            }
            for (const auto& version_dir : fs::directory_iterator(name_dir.path()))
            {
                if (!version_dir.is_directory())
                {
                    continue;
                }
                for (const auto& user_dir : fs::directory_iterator(version_dir.path()))
                {
                    if (!user_dir.is_directory())
                    {
                        continue;
                    }
                    for (const auto& channel_dir : fs::directory_iterator(user_dir.path()))
                    {
                        if (!channel_dir.is_directory())
                        {
                            continue;
                        }
                        refs.push_back({name_dir.path().filename().string(),
                                        version_dir.path().filename().string(),
                                        user_dir.path().filename().string(),
                                        channel_dir.path().filename().string()});
                    }
                }
            }
        }
        std::sort(refs.begin(),
                  refs.end(),
                  [](const RecipeRef& lhs, const RecipeRef& rhs)
                  {
                      return std::tie(lhs.name, lhs.version, lhs.user, lhs.channel) <
                             std::tie(rhs.name, rhs.version, rhs.user, rhs.channel);
                  });
        return refs;
    }

  private:
    [[nodiscard]] std::vector<RevisionInfo> list_revisions(const fs::path& revisions_dir) const
    {
        std::vector<RevisionInfo> revisions;
        if (!fs::exists(revisions_dir))
        {
            return revisions;
        }
        for (const auto& entry : fs::directory_iterator(revisions_dir))
        {
            if (!entry.is_directory())
            {
                continue;
            }
            const fs::path info_path = entry.path() / "revision.json";
            std::string    time      = iso8601_now();
            if (std::ifstream in(info_path); in)
            {
                std::string line;
                std::getline(in, line);
                if (!trim(line).empty())
                {
                    time = trim(line);
                }
            }
            revisions.push_back({entry.path().filename().string(), time, entry.path()});
        }
        std::sort(revisions.begin(),
                  revisions.end(),
                  [](const RevisionInfo& lhs, const RevisionInfo& rhs)
                  { return lhs.time > rhs.time; });
        return revisions;
    }

    fs::path root_;
};

class AuthManager
{
  public:
    explicit AuthManager(User admin) : admin_(std::move(admin))
    {
    }

    [[nodiscard]] bool valid_basic(const std::string& username, const std::string& password) const
    {
        return username == admin_.username && password == admin_.password;
    }

    [[nodiscard]] std::optional<std::string> authenticate(const httplib::Request& req)
    {
        const auto authorization = req.get_header_value("Authorization");
        if (!authorization.empty())
        {
            if (const auto basic = parse_basic_header(authorization);
                basic && valid_basic(basic->first, basic->second))
            {
                return basic->first;
            }
            if (const auto bearer = parse_bearer_header(authorization))
            {
                const auto it = sessions_.find(*bearer);
                if (it != sessions_.end() &&
                    it->second.expires_at > std::chrono::system_clock::now())
                {
                    return it->second.username;
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string issue_token(const std::string& username)
    {
        Session session{
            username, random_token(40), std::chrono::system_clock::now() + std::chrono::hours(24)};
        sessions_[session.token] = session;
        return session.token;
    }

  private:
    User                                     admin_;
    std::unordered_map<std::string, Session> sessions_;
};

std::string load_or_generate_password(const fs::path& config_path)
{
    if (std::ifstream in(config_path); in)
    {
        std::string line;
        while (std::getline(in, line))
        {
            const auto sep = line.find('=');
            if (sep == std::string::npos)
            {
                continue;
            }
            const std::string key   = trim(line.substr(0, sep));
            const std::string value = trim(line.substr(sep + 1));
            if (key == "admin_password" && !value.empty())
            {
                return value;
            }
        }
    }
    return random_token(16);
}

ServerConfig load_config(const fs::path& storage_root)
{
    PackageStorage storage(storage_root);
    storage.ensure_layout();

    ServerConfig config;
    config.storage_root   = storage_root;
    config.admin_password = load_or_generate_password(storage.config_path());

    if (std::ifstream in(storage.config_path()); in)
    {
        std::string line;
        while (std::getline(in, line))
        {
            const auto sep = line.find('=');
            if (sep == std::string::npos)
            {
                continue;
            }
            const std::string key   = trim(line.substr(0, sep));
            const std::string value = trim(line.substr(sep + 1));
            if (key == "host" && !value.empty())
            {
                config.host = value;
            }
            else if (key == "port" && !value.empty())
            {
                config.port = std::stoi(value);
            }
            else if (key == "admin_user" && !value.empty())
            {
                config.admin_user = value;
            }
            else if (key == "admin_password" && !value.empty())
            {
                config.admin_password = value;
            }
        }
    }
    else
    {
        std::ofstream out(storage.config_path(), std::ios::trunc);
        out << "host=" << config.host << '\n';
        out << "port=" << config.port << '\n';
        out << "admin_user=" << config.admin_user << '\n';
        out << "admin_password=" << config.admin_password << '\n';
    }

    return config;
}

std::optional<RecipeRef> make_ref(const httplib::Match& matches, std::size_t offset = 1)
{
    if (matches.size() < offset + 4)
    {
        return std::nullopt;
    }
    RecipeRef ref{matches[offset].str(),
                  matches[offset + 1].str(),
                  matches[offset + 2].str(),
                  matches[offset + 3].str()};
    if (!is_safe_path_segment(ref.name) || !is_safe_path_segment(ref.version) ||
        !is_safe_path_segment(ref.user) || !is_safe_path_segment(ref.channel))
    {
        return std::nullopt;
    }
    return ref;
}

std::optional<std::string> read_small_file(const fs::path& file)
{
    std::ifstream in(platform_fs_path(file), std::ios::binary);
    if (!in)
    {
        return std::nullopt;
    }
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

std::map<std::string, std::map<std::string, std::string>>
parse_ini_sections(const std::string& content)
{
    std::map<std::string, std::map<std::string, std::string>> sections;
    std::string                                               current = "root";
    std::istringstream                                        input(content);
    std::string                                               line;
    while (std::getline(input, line))
    {
        line = trim(line);
        if (line.empty() || line.starts_with('#') || line.starts_with(';'))
        {
            continue;
        }
        if (line.front() == '[' && line.back() == ']')
        {
            current = line.substr(1, line.size() - 2);
            continue;
        }
        const auto sep = line.find('=');
        if (sep == std::string::npos)
        {
            continue;
        }
        sections[current][trim(line.substr(0, sep))] = trim(line.substr(sep + 1));
    }
    return sections;
}

std::string package_search_json(const fs::path& conaninfo_file)
{
    const auto         info   = read_small_file(conaninfo_file).value_or("");
    const auto         parsed = parse_ini_sections(info);
    std::ostringstream out;
    out << '{';
    bool first_section = true;
    auto append_object =
        [&](const std::string& label, const std::map<std::string, std::string>& values)
    {
        if (!first_section)
        {
            out << ',';
        }
        first_section = false;
        out << '"' << json_escape(label) << "\":{";
        bool first = true;
        for (const auto& [key, value] : values)
        {
            if (!first)
            {
                out << ',';
            }
            first = false;
            out << '"' << json_escape(key) << "\":\"" << json_escape(value) << '"';
        }
        out << '}';
    };
    const auto settings         = parsed.find("settings");
    const auto options          = parsed.find("options");
    const auto requires_section = parsed.find("requires");
    const auto recipe_hash      = parsed.find("recipe_hash");
    if (settings != parsed.end())
    {
        append_object("settings", settings->second);
    }
    if (options != parsed.end())
    {
        append_object("options", options->second);
    }
    if (requires_section != parsed.end())
    {
        append_object("requires", requires_section->second);
    }
    if (recipe_hash != parsed.end())
    {
        append_object("recipe_hash", recipe_hash->second);
    }
    out << '}';
    return out.str();
}

std::string summary_json(const PackageStorage& storage)
{
    const auto  refs            = storage.list_recipe_refs();
    std::size_t total_revisions = 0;
    std::size_t total_packages  = 0;
    for (const auto& ref : refs)
    {
        const auto revisions = storage.list_recipe_revisions(ref);
        total_revisions += revisions.size();
        for (const auto& revision : revisions)
        {
            const fs::path packages_dir = revision.path / "packages";
            if (!fs::exists(packages_dir))
            {
                continue;
            }
            for (const auto& package_dir : fs::directory_iterator(packages_dir))
            {
                if (package_dir.is_directory())
                {
                    ++total_packages;
                }
            }
        }
    }
    std::ostringstream out;
    out << "{"
        << "\"recipes\":" << refs.size() << ',' << "\"recipe_revisions\":" << total_revisions << ','
        << "\"packages\":" << total_packages << ',' << "\"storage\":\""
        << json_escape(storage.root().string()) << "\"}";
    return out.str();
}

std::string recipes_json(const PackageStorage& storage)
{
    const auto         refs = storage.list_recipe_refs();
    std::ostringstream out;
    out << "{\"recipes\":[";
    bool first_ref = true;
    for (const auto& ref : refs)
    {
        if (!first_ref)
        {
            out << ',';
        }
        first_ref            = false;
        const auto revisions = storage.list_recipe_revisions(ref);
        out << '{';
        out << "\"reference\":\""
            << json_escape(ref.name + "/" + ref.version + "@" + ref.user + "/" + ref.channel)
            << "\",";
        out << "\"name\":\"" << json_escape(ref.name) << "\",";
        out << "\"version\":\"" << json_escape(ref.version) << "\",";
        out << "\"user\":\"" << json_escape(ref.user) << "\",";
        out << "\"channel\":\"" << json_escape(ref.channel) << "\",";
        out << "\"revisions\":[";
        bool first_revision = true;
        for (const auto& revision : revisions)
        {
            if (!first_revision)
            {
                out << ',';
            }
            first_revision = false;
            out << '{';
            out << "\"revision\":\"" << json_escape(revision.revision) << "\",";
            out << "\"time\":\"" << json_escape(revision.time) << "\",";
            out << "\"files\":" << storage.list_files(revision.path / "files").size() << ',';
            std::size_t    package_refs = 0;
            const fs::path packages_dir = revision.path / "packages";
            if (fs::exists(packages_dir))
            {
                for (const auto& entry : fs::directory_iterator(packages_dir))
                {
                    if (entry.is_directory())
                    {
                        ++package_refs;
                    }
                }
            }
            out << "\"package_refs\":" << package_refs;
            out << '}';
        }
        out << "]}";
    }
    out << "]}";
    return out.str();
}

void add_capability_headers(httplib::Response& res)
{
    res.set_header("X-Conan-Server-Capabilities", "revisions");
    res.set_header("Server", "LeafConan/0.1");
}

void set_json(httplib::Response& res, const std::string& body, int status = 200)
{
    add_capability_headers(res);
    res.status = status;
    res.set_content(body, "application/json");
}

void set_plain(httplib::Response& res, const std::string& body, int status = 200)
{
    add_capability_headers(res);
    res.status = status;
    res.set_content(body, "text/plain; charset=utf-8");
}

void set_unauthorized(httplib::Response& res)
{
    add_capability_headers(res);
    res.status = 401;
    res.set_header("WWW-Authenticate", "Basic realm=\"Leaf Conan Server\"");
    res.set_content("Unauthorized", "text/plain; charset=utf-8");
}

template <typename Handler>
void with_auth(AuthManager&            auth,
               Handler&&               handler,
               const httplib::Request& req,
               httplib::Response&      res)
{
    if (const auto username = auth.authenticate(req))
    {
        handler(*username);
        return;
    }
    set_unauthorized(res);
}

template <typename Handler>
void allow_anonymous_or_auth(AuthManager& auth, Handler&& handler, const httplib::Request& req)
{
    const auto username = auth.authenticate(req);
    handler(username);
}

template <typename Handler>
void allow_anonymous_or_auth(AuthManager&            auth,
                             Handler&&               handler,
                             const httplib::Request& req,
                             httplib::Response&)
{
    allow_anonymous_or_auth(auth, std::forward<Handler>(handler), req);
}

bool ensure_parent_dir(const fs::path& file)
{
    std::error_code ec;
    fs::create_directories(platform_fs_path(file).parent_path(), ec);
    return !ec;
}

void write_revision_info(const fs::path& revision_dir)
{
    std::ofstream out(revision_dir / "revision.json", std::ios::trunc);
    out << iso8601_now() << '\n';
}

std::string file_listing_json(const std::vector<std::string>& files)
{
    std::ostringstream out;
    out << "{\"files\":{";
    bool first = true;
    for (const auto& file : files)
    {
        if (!first)
        {
            out << ',';
        }
        first = false;
        out << '"' << json_escape(file) << "\":{}";
    }
    out << "}}";
    return out.str();
}

void handle_file_get(const fs::path& file_path, httplib::Response& res)
{
    std::ifstream in(platform_fs_path(file_path), std::ios::binary);
    if (!in)
    {
        set_plain(res, "Not Found", 404);
        return;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    add_capability_headers(res);
    res.status = 200;
    res.set_content(buffer.str(), "application/octet-stream");
}

void handle_body_upload(const fs::path&    file_path,
                        const fs::path&    revision_dir,
                        const std::string& body,
                        httplib::Response& res)
{
    try
    {
        append_debug_log("HANDLE_UPLOAD path=" + file_path.string() +
                         " body=" + std::to_string(body.size()));
        if (!ensure_parent_dir(file_path))
        {
            append_debug_log("HANDLE_UPLOAD prepare_parent_failed");
            set_plain(res, "Unable to prepare storage", 500);
            return;
        }

        std::ofstream out(platform_fs_path(file_path), std::ios::binary | std::ios::trunc);
        if (!out)
        {
            append_debug_log("HANDLE_UPLOAD open_failed");
            set_plain(res, "Unable to open destination file", 500);
            return;
        }

        out.write(body.data(), static_cast<std::streamsize>(body.size()));
        out.close();

        if (!out.good())
        {
            append_debug_log("HANDLE_UPLOAD write_failed");
            set_plain(res, "Upload failed", 500);
            return;
        }

        std::error_code ec;
        fs::create_directories(platform_fs_path(revision_dir), ec);
        if (ec)
        {
            append_debug_log("HANDLE_UPLOAD revision_dir_failed " + ec.message());
            set_plain(res, "Unable to prepare revision metadata", 500);
            return;
        }
        write_revision_info(revision_dir);
        append_debug_log("HANDLE_UPLOAD ok");
        set_json(res, "{\"status\":\"ok\"}");
    }
    catch (const std::exception& ex)
    {
        append_debug_log(std::string("HANDLE_UPLOAD exception=") + ex.what());
        set_plain(res, std::string("Upload exception: ") + ex.what(), 500);
    }
}

void add_recipe_routes(httplib::Server& app, PackageStorage& storage, AuthManager& auth)
{
    app.Get("/v1/ping",
            [&](const httplib::Request&, httplib::Response& res) { set_plain(res, ""); });
    app.Get("/v2/ping",
            [&](const httplib::Request&, httplib::Response& res) { set_plain(res, ""); });

    app.Get("/v1/users/authenticate",
            [&](const httplib::Request& req, httplib::Response& res)
            {
                if (auth.authenticate(req))
                {
                    set_plain(res, auth.issue_token("admin"));
                }
                else
                {
                    set_unauthorized(res);
                }
            });

    app.Get("/v2/users/authenticate",
            [&](const httplib::Request& req, httplib::Response& res)
            {
                if (const auto username = auth.authenticate(req))
                {
                    set_plain(res, auth.issue_token(*username));
                }
                else
                {
                    set_unauthorized(res);
                }
            });

    app.Get("/v2/users/check_credentials",
            [&](const httplib::Request& req, httplib::Response& res)
            {
                if (auth.authenticate(req))
                {
                    set_plain(res, "ok");
                }
                else
                {
                    set_unauthorized(res);
                }
            });

    app.Get("/api/ui/login",
            [&](const httplib::Request& req, httplib::Response& res)
            {
                if (const auto username = auth.authenticate(req))
                {
                    set_json(res,
                             "{\"token\":\"" + json_escape(auth.issue_token(*username)) + "\"}");
                }
                else
                {
                    set_unauthorized(res);
                }
            });

    app.Get(
        "/api/ui/summary",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            with_auth(
                auth, [&](const std::string&) { set_json(res, summary_json(storage)); }, req, res);
        });

    app.Get(
        "/api/ui/recipes",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            with_auth(
                auth, [&](const std::string&) { set_json(res, recipes_json(storage)); }, req, res);
        });

    app.Get("/v2/conans/search",
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const std::string query    = req.get_param_value("q");
                        const std::string wildcard = query.empty() ? "*" : query;
                        const std::string regex_pattern =
                            "^" +
                            std::regex_replace(
                                wildcard, std::regex(R"([\.\+\-\[\]\(\)\{\}\^\$\|])"), R"(\$&)") +
                            "$";
                        std::string normalized = regex_pattern;
                        std::replace(normalized.begin(), normalized.end(), '*', '.');
                        normalized = std::regex_replace(normalized, std::regex(R"(\.)"), ".*");
                        const std::regex   matcher(normalized, std::regex::icase);
                        std::ostringstream out;
                        out << "{\"results\":[";
                        bool first = true;
                        for (const auto& ref : storage.list_recipe_refs())
                        {
                            const std::string value =
                                ref.name + "/" + ref.version + "@" + ref.user + "/" + ref.channel;
                            if (!std::regex_match(value, matcher))
                            {
                                continue;
                            }
                            if (!first)
                            {
                                out << ',';
                            }
                            first = false;
                            out << '"' << json_escape(value) << '"';
                        }
                        out << "]}";
                        set_json(res, out.str());
                    },
                    req,
                    res);
            });

    const auto recipe_prefix = R"(/v2/conans/([^/]+)/([^/]+)/([^/]+)/([^/]+))";
    app.Get(recipe_prefix + std::string(R"(/latest)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto ref = make_ref(req.matches);
                        if (!ref)
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        const auto revisions = storage.list_recipe_revisions(*ref);
                        if (revisions.empty())
                        {
                            set_plain(res, "Not Found", 404);
                            return;
                        }
                        set_json(res,
                                 "{\"revision\":\"" + json_escape(revisions.front().revision) +
                                     "\",\"time\":\"" + json_escape(revisions.front().time) +
                                     "\"}");
                    },
                    req,
                    res);
            });

    app.Get(recipe_prefix + std::string(R"(/revisions)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto ref = make_ref(req.matches);
                        if (!ref)
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        std::ostringstream out;
                        out << "{\"reference\":\""
                            << json_escape(ref->name + "/" + ref->version + "@" + ref->user + "/" +
                                           ref->channel)
                            << "\",\"revisions\":[";
                        bool first = true;
                        for (const auto& revision : storage.list_recipe_revisions(*ref))
                        {
                            if (!first)
                            {
                                out << ',';
                            }
                            first = false;
                            out << "{\"revision\":\"" << json_escape(revision.revision)
                                << "\",\"time\":\"" << json_escape(revision.time) << "\"}";
                        }
                        out << "]}";
                        set_json(res, out.str());
                    },
                    req,
                    res);
            });

    app.Delete(recipe_prefix + std::string(R"(/revisions/([^/]+))"),
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   with_auth(
                       auth,
                       [&](const std::string&)
                       {
                           const auto        ref      = make_ref(req.matches);
                           const std::string revision = req.matches[5].str();
                           if (!ref || !is_safe_path_segment(revision))
                           {
                               set_plain(res, "Invalid reference", 400);
                               return;
                           }
                           const fs::path revision_dir =
                               storage.recipe_revision_path(*ref, revision);
                           if (!fs::exists(revision_dir))
                           {
                               set_plain(res, "Not Found", 404);
                               return;
                           }
                           std::error_code ec;
                           fs::remove_all(revision_dir, ec);
                           if (ec)
                           {
                               set_plain(res, "Delete failed", 500);
                               return;
                           }
                           set_json(res, "{\"status\":\"deleted\"}");
                       },
                       req,
                       res);
               });

    app.Get(recipe_prefix + std::string(R"(/revisions/([^/]+)/files)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto        ref      = make_ref(req.matches);
                        const std::string revision = req.matches[5].str();
                        if (!ref || !is_safe_path_segment(revision))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        const auto files =
                            storage.list_files(storage.recipe_files_path(*ref, revision));
                        if (files.empty() &&
                            !fs::exists(storage.recipe_revision_path(*ref, revision)))
                        {
                            set_plain(res, "Not Found", 404);
                            return;
                        }
                        set_json(res, file_listing_json(files));
                    },
                    req);
            });

    app.Get(recipe_prefix + std::string(R"(/revisions/([^/]+)/files/([^/]+))"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto        ref       = make_ref(req.matches);
                        const std::string revision  = req.matches[5].str();
                        const std::string file_name = req.matches[6].str();
                        if (!ref || !is_safe_path_segment(revision) ||
                            !is_safe_path_segment(file_name))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        handle_file_get(storage.recipe_files_path(*ref, revision) / file_name, res);
                    },
                    req);
            });

    app.Put(recipe_prefix + std::string(R"(/revisions/([^/]+)/files/([^/]+))"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                with_auth(
                    auth,
                    [&](const std::string&)
                    {
                        const auto        ref       = make_ref(req.matches);
                        const std::string revision  = req.matches[5].str();
                        const std::string file_name = req.matches[6].str();
                        if (!ref || !is_safe_path_segment(revision) ||
                            !is_safe_path_segment(file_name))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        const fs::path revision_dir = storage.recipe_revision_path(*ref, revision);
                        handle_body_upload(storage.recipe_files_path(*ref, revision) / file_name,
                                           revision_dir,
                                           req.body,
                                           res);
                    },
                    req,
                    res);
            });

    app.Get(recipe_prefix + std::string(R"(/search)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto ref = make_ref(req.matches);
                        if (!ref)
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        std::ostringstream out;
                        out << '{';
                        bool       first_package = true;
                        const auto revisions     = storage.list_recipe_revisions(*ref);
                        if (!revisions.empty())
                        {
                            const fs::path packages_dir = revisions.front().path / "packages";
                            if (fs::exists(packages_dir))
                            {
                                for (const auto& package_dir : fs::directory_iterator(packages_dir))
                                {
                                    if (!package_dir.is_directory())
                                    {
                                        continue;
                                    }
                                    const std::string package_id =
                                        package_dir.path().filename().string();
                                    const auto package_revisions = storage.list_package_revisions(
                                        *ref, revisions.front().revision, package_id);
                                    if (package_revisions.empty())
                                    {
                                        continue;
                                    }
                                    const fs::path conaninfo =
                                        package_revisions.front().path / "files" / "conaninfo.txt";
                                    if (!first_package)
                                    {
                                        out << ',';
                                    }
                                    first_package = false;
                                    out << '"' << json_escape(package_id)
                                        << "\":" << package_search_json(conaninfo);
                                }
                            }
                        }
                        out << '}';
                        set_json(res, out.str());
                    },
                    req);
            });

    app.Get(recipe_prefix + std::string(R"(/revisions/([^/]+)/search)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto        ref             = make_ref(req.matches);
                        const std::string recipe_revision = req.matches[5].str();
                        if (!ref || !is_safe_path_segment(recipe_revision))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        std::ostringstream out;
                        out << '{';
                        bool           first_package = true;
                        const fs::path packages_dir =
                            storage.recipe_revision_path(*ref, recipe_revision) / "packages";
                        if (fs::exists(packages_dir))
                        {
                            for (const auto& package_dir : fs::directory_iterator(packages_dir))
                            {
                                if (!package_dir.is_directory())
                                {
                                    continue;
                                }
                                const std::string package_id =
                                    package_dir.path().filename().string();
                                const auto package_revisions = storage.list_package_revisions(
                                    *ref, recipe_revision, package_id);
                                if (package_revisions.empty())
                                {
                                    continue;
                                }
                                const fs::path conaninfo =
                                    package_revisions.front().path / "files" / "conaninfo.txt";
                                if (!first_package)
                                {
                                    out << ',';
                                }
                                first_package = false;
                                out << '"' << json_escape(package_id)
                                    << "\":" << package_search_json(conaninfo);
                            }
                        }
                        out << '}';
                        set_json(res, out.str());
                    },
                    req);
            });

    const auto package_prefix =
        recipe_prefix + std::string(R"(/revisions/([^/]+)/packages/([^/]+))");

    app.Get(package_prefix + std::string(R"(/latest)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto        ref             = make_ref(req.matches);
                        const std::string recipe_revision = req.matches[5].str();
                        const std::string package_id      = req.matches[6].str();
                        if (!ref || !is_safe_path_segment(recipe_revision) ||
                            !is_safe_path_segment(package_id))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        const auto revisions =
                            storage.list_package_revisions(*ref, recipe_revision, package_id);
                        if (revisions.empty())
                        {
                            set_plain(res, "Not Found", 404);
                            return;
                        }
                        set_json(res,
                                 "{\"revision\":\"" + json_escape(revisions.front().revision) +
                                     "\",\"time\":\"" + json_escape(revisions.front().time) +
                                     "\"}");
                    },
                    req);
            });

    app.Get(package_prefix + std::string(R"(/revisions)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto        ref             = make_ref(req.matches);
                        const std::string recipe_revision = req.matches[5].str();
                        const std::string package_id      = req.matches[6].str();
                        if (!ref || !is_safe_path_segment(recipe_revision) ||
                            !is_safe_path_segment(package_id))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        std::ostringstream out;
                        out << "{\"reference\":\""
                            << json_escape(ref->name + "/" + ref->version + "@" + ref->user + "/" +
                                           ref->channel + "#" + recipe_revision + ":" + package_id)
                            << "\",\"revisions\":[";
                        bool first = true;
                        for (const auto& revision :
                             storage.list_package_revisions(*ref, recipe_revision, package_id))
                        {
                            if (!first)
                            {
                                out << ',';
                            }
                            first = false;
                            out << "{\"revision\":\"" << json_escape(revision.revision)
                                << "\",\"time\":\"" << json_escape(revision.time) << "\"}";
                        }
                        out << "]}";
                        set_json(res, out.str());
                    },
                    req);
            });

    app.Delete(package_prefix + std::string(R"(/revisions/([^/]+))"),
               [&](const httplib::Request& req, httplib::Response& res)
               {
                   with_auth(
                       auth,
                       [&](const std::string&)
                       {
                           const auto        ref              = make_ref(req.matches);
                           const std::string recipe_revision  = req.matches[5].str();
                           const std::string package_id       = req.matches[6].str();
                           const std::string package_revision = req.matches[7].str();
                           if (!ref || !is_safe_path_segment(recipe_revision) ||
                               !is_safe_path_segment(package_id) ||
                               !is_safe_path_segment(package_revision))
                           {
                               set_plain(res, "Invalid reference", 400);
                               return;
                           }
                           const fs::path revision_dir = storage.package_revision_path(
                               *ref, recipe_revision, package_id, package_revision);
                           if (!fs::exists(revision_dir))
                           {
                               set_plain(res, "Not Found", 404);
                               return;
                           }
                           std::error_code ec;
                           fs::remove_all(revision_dir, ec);
                           if (ec)
                           {
                               set_plain(res, "Delete failed", 500);
                               return;
                           }
                           set_json(res, "{\"status\":\"deleted\"}");
                       },
                       req,
                       res);
               });

    app.Get(package_prefix + std::string(R"(/revisions/([^/]+)/files)"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto        ref              = make_ref(req.matches);
                        const std::string recipe_revision  = req.matches[5].str();
                        const std::string package_id       = req.matches[6].str();
                        const std::string package_revision = req.matches[7].str();
                        if (!ref || !is_safe_path_segment(recipe_revision) ||
                            !is_safe_path_segment(package_id) ||
                            !is_safe_path_segment(package_revision))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        const auto files = storage.list_files(storage.package_files_path(
                            *ref, recipe_revision, package_id, package_revision));
                        if (files.empty() &&
                            !fs::exists(storage.package_revision_path(
                                *ref, recipe_revision, package_id, package_revision)))
                        {
                            set_plain(res, "Not Found", 404);
                            return;
                        }
                        set_json(res, file_listing_json(files));
                    },
                    req);
            });

    app.Get(package_prefix + std::string(R"(/revisions/([^/]+)/files/([^/]+))"),
            [&](const httplib::Request& req, httplib::Response& res)
            {
                allow_anonymous_or_auth(
                    auth,
                    [&](const std::optional<std::string>&)
                    {
                        const auto        ref              = make_ref(req.matches);
                        const std::string recipe_revision  = req.matches[5].str();
                        const std::string package_id       = req.matches[6].str();
                        const std::string package_revision = req.matches[7].str();
                        const std::string file_name        = req.matches[8].str();
                        if (!ref || !is_safe_path_segment(recipe_revision) ||
                            !is_safe_path_segment(package_id) ||
                            !is_safe_path_segment(package_revision) ||
                            !is_safe_path_segment(file_name))
                        {
                            set_plain(res, "Invalid reference", 400);
                            return;
                        }
                        handle_file_get(storage.package_files_path(
                                            *ref, recipe_revision, package_id, package_revision) /
                                            file_name,
                                        res);
                    },
                    req);
            });

    app.Put(
        package_prefix + std::string(R"(/revisions/([^/]+)/files/([^/]+))"),
        [&](const httplib::Request& req, httplib::Response& res)
        {
            with_auth(
                auth,
                [&](const std::string&)
                {
                    const auto        ref              = make_ref(req.matches);
                    const std::string recipe_revision  = req.matches[5].str();
                    const std::string package_id       = req.matches[6].str();
                    const std::string package_revision = req.matches[7].str();
                    const std::string file_name        = req.matches[8].str();
                    if (!ref || !is_safe_path_segment(recipe_revision) ||
                        !is_safe_path_segment(package_id) ||
                        !is_safe_path_segment(package_revision) || !is_safe_path_segment(file_name))
                    {
                        set_plain(res, "Invalid reference", 400);
                        return;
                    }
                    const fs::path revision_dir = storage.package_revision_path(
                        *ref, recipe_revision, package_id, package_revision);
                    handle_body_upload(storage.package_files_path(
                                           *ref, recipe_revision, package_id, package_revision) /
                                           file_name,
                                       revision_dir,
                                       req.body,
                                       res);
                },
                req,
                res);
        });
}

void print_usage()
{
    std::cout
        << "leafserver usage:\n"
        << "  leaf run leafserver -- [--host 0.0.0.0] [--port 9300] [--storage .leafserver-data]\n"
        << "  Conan remote URL example: http://127.0.0.1:9300\n";
}

} // namespace

int run(int argc, char** argv)
{
    fs::path                   storage_root = ".leafserver-data";
    std::optional<std::string> host_override;
    std::optional<int>         port_override;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h")
        {
            print_usage();
            return 0;
        }
        if (arg == "--host" && i + 1 < argc)
        {
            host_override = argv[++i];
            continue;
        }
        if (arg == "--port" && i + 1 < argc)
        {
            port_override = std::stoi(argv[++i]);
            continue;
        }
        if (arg == "--storage" && i + 1 < argc)
        {
            storage_root = argv[++i];
            continue;
        }
    }

    ServerConfig config = load_config(storage_root);
    if (host_override)
    {
        config.host = *host_override;
    }
    if (port_override)
    {
        config.port = *port_override;
    }

    PackageStorage storage(config.storage_root);
    AuthManager    auth(User{config.admin_user, config.admin_password});
    g_debug_log_path = config.storage_root / "server-debug.log";
    std::error_code remove_ec;
    fs::remove(g_debug_log_path, remove_ec);

    httplib::Server app;

    app.set_keep_alive_max_count(100);
    app.set_keep_alive_timeout(10);
    app.set_read_timeout(300, 0);
    app.set_write_timeout(300, 0);
    app.set_payload_max_length(1024ULL * 1024ULL * 1024ULL);
    app.set_pre_request_handler([](const httplib::Request&, httplib::Response&)
                                { return httplib::Server::HandlerResponse::Unhandled; });
    app.set_pre_routing_handler(
        [&](const httplib::Request& req, httplib::Response&)
        {
            if (req.method == "PUT")
            {
                append_debug_log(
                    "PUT " + req.path + " len=" + std::to_string(req.body.size()) +
                    " expect=" + req.get_header_value("Expect") +
                    " te=" + req.get_header_value("Transfer-Encoding") +
                    " cl=" + req.get_header_value("Content-Length") + " auth=" +
                    (req.has_header("Authorization") ? std::string("yes") : std::string("no")) +
                    " ce=" + req.get_header_value("Content-Encoding"));
            }
            return httplib::Server::HandlerResponse::Unhandled;
        });
    app.set_logger(
        [&](const httplib::Request& req, const httplib::Response& res)
        {
            if (req.method == "PUT")
            {
                append_debug_log("PUT_DONE " + req.path + " -> " + std::to_string(res.status));
            }
        });
    app.set_error_logger(
        [&](const httplib::Error& error, const httplib::Request* req)
        {
            std::string line = "HTTP_ERROR code=" + std::to_string(static_cast<int>(error));
            if (req != nullptr)
            {
                line += " path=" + req->path;
            }
            append_debug_log(line);
        });
    app.set_exception_handler(
        [&](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep)
        {
            try
            {
                if (ep)
                {
                    std::rethrow_exception(ep);
                }
            }
            catch (const std::exception& ex)
            {
                append_debug_log("EXCEPTION path=" + req.path + " message=" + ex.what());
                set_plain(res, std::string("Exception: ") + ex.what(), 500);
                return;
            }
            catch (...)
            {
                append_debug_log("EXCEPTION path=" + req.path + " message=unknown");
                set_plain(res, "Exception: unknown", 500);
                return;
            }
            set_plain(res, "Exception: unknown", 500);
        });
    add_recipe_routes(app, storage, auth);

    httplib::mount(app, Web::FS);

    app.set_error_handler(
        [](const httplib::Request&, httplib::Response& res)
        {
            add_capability_headers(res);
            if (res.status == 404)
            {
                res.set_content("Not Found", "text/plain; charset=utf-8");
            }
        });

    std::cout << "Leaf Conan Server starting\n"
              << "  host: " << config.host << '\n'
              << "  port: " << config.port << '\n'
              << "  storage: " << fs::absolute(config.storage_root).string() << '\n'
              << "  admin user: " << config.admin_user << '\n'
              << "  admin password: " << config.admin_password << '\n'
              << "  remote url: http://" << (config.host == "0.0.0.0" ? "127.0.0.1" : config.host)
              << ':' << config.port << std::endl;

    if (!app.listen(config.host, config.port))
    {
        std::cerr << "Failed to bind server on " << config.host << ':' << config.port << '\n';
        return 1;
    }

    return 0;
}

} // namespace server
