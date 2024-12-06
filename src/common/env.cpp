#include "env.h"

#include <fstream>
#include <map>
#include <windows.h>
#include <PathCch.h>
#include <shlobj.h>
#include <vector>

#include "util.h"

using namespace std::literals;

static Path get_env_var (const wchar_t *name) {
    wchar_t *path;
    _wdupenv_s(&path, nullptr, name);
    std::wstring result = path;
    free(path);

    // if path do not end with '\' or '/', append it
    if (result.back() != L'\\' && result.back() != L'/') {
        result += L'\\';
    }

    return std::filesystem::absolute(result);
}

static Path get_folder_path (_In_ REFKNOWNFOLDERID rfid) {
    PWSTR path;
    HRESULT result = SHGetKnownFolderPath(rfid, 0, nullptr, &path);

    if (result != S_OK) {
        throw std::runtime_error("Failed to get known folder path");
    }

    std::wstring wpath = path;
    CoTaskMemFree(path);

    // if path do not end with '\' or '/', append it
    if (wpath.back() != L'\\' && wpath.back() != L'/') {
        wpath += L'\\';
    }

    return std::filesystem::absolute(wpath);
}

SystemFolders SystemFolders::default_system_folders () {
    SystemFolders folders;

    // User Folders =============================================================

    folders.user_profile = get_env_var(L"USERPROFILE");

    folders.desktop = get_folder_path(FOLDERID_Desktop);
    folders.documents = get_folder_path(FOLDERID_Documents);
    folders.downloads = get_folder_path(FOLDERID_Downloads);
    folders.music = get_folder_path(FOLDERID_Music);
    folders.pictures = get_folder_path(FOLDERID_Pictures);
    folders.videos = get_folder_path(FOLDERID_Videos);
    folders.saved_games = get_folder_path(FOLDERID_SavedGames);

    // Program Folders ==========================================================

    folders.roaming_app_data = get_folder_path(FOLDERID_RoamingAppData);
    folders.local_app_data = get_folder_path(FOLDERID_LocalAppData);
    folders.local_low_app_data = get_folder_path(FOLDERID_LocalAppDataLow);


    return folders;
}


std::map<String, Path> SystemFolders::to_map () const {
    return {
        {L"USER_PROFILE", user_profile},

        {L"DESKTOP", desktop},
        {L"DOCUMENTS", documents},
        {L"DOWNLOADS", downloads},
        {L"MUSIC", music},
        {L"PICTURES", pictures},
        {L"VIDEOS", videos},
        {L"SAVED_GAMES", saved_games},

        {L"ROAMING_APP_DATA", roaming_app_data},
        {L"LOCAL_APP_DATA", local_app_data},
        {L"LOCAL_LOW_APP_DATA", local_low_app_data}
    };
}

const std::vector<String> &SystemFolders::vars () {
    static std::vector<String> vars = {
        L"USER_PROFILE",

        L"DESKTOP",
        L"DOCUMENTS",
        L"DOWNLOADS",
        L"MUSIC",
        L"PICTURES",
        L"VIDEOS",
        L"SAVED_GAMES",

        L"ROAMING_APP_DATA",
        L"LOCAL_APP_DATA",
        L"LOCAL_LOW_APP_DATA"
    };
    return vars;
}

std::map<String, Path> EnvFolders::to_map () const {
    return {
        {L"GAME", original_game_folder}
    };
}

const std::vector<String> &EnvFolders::vars () {
    static std::vector<String> vars = {
        L"GAME"
    };
    return vars;
}


struct TranslatedRule {
    Path src;
    Path dst;
};


static TranslatedRule translate_rule (const Env &env, const Rule &rule) {
    Path src;

    auto base_paths = env.system_folders.to_map();
    base_paths.merge(env.env_folders.to_map());

    if (!rule.src_base.empty()) {
        if (base_paths.count(rule.src_base) == 0) {
            throw std::runtime_error("Unknown base path");
        }

        if (rule.src.empty()) {
            src = base_paths[rule.src_base];
        } else {
            // concat src with src_relative
            src = base_paths[rule.src_base] / rule.src;
        }
    } else {
        src = rule.src;
    }

    Path dst = (env.env_folders.game_folder / rule.dst_name).wstring() + L"\\";

    return TranslatedRule{src, dst};
}

static std::wstring to_lower (const std::wstring &s) {
    std::wstring result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::towlower);
    return result;
}


static bool apply_rule_inner (const Path &path, const TranslatedRule &rule, Path &result) {
    // disk letter must be the same
#ifdef DEBUG
    std::wstring debug_info = L"@MilkFeng apply_rule_inner: "s + path.wstring() + L" "s + rule.src.wstring() + L" "s + rule.dst.wstring() + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    Path path_ = to_lower(path.wstring());
    Path src_ = to_lower(rule.src.wstring());
    Path dst_ = to_lower(rule.dst.wstring());

#ifdef DEBUG
    debug_info = L"@MilkFeng apply_rule_inner 3: "s + absolute(path_).wstring() + L" " + absolute(src_).wstring() + L"\n"s;
    OutputDebugStringW(debug_info.c_str());

    debug_info = L"@MilkFeng apply_rule_inner 3.1: "s + std::to_wstring(absolute(path_).wstring().find(absolute(src_).wstring())) + L" "s + std::to_wstring(std::wstring::npos) + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    // if path is not in src, return false
    if (absolute(path_).wstring().find(absolute(src_).wstring()) == std::wstring::npos) {
#ifdef DEBUG
        OutputDebugStringW(L"@MilkFeng apply_rule_inner 3.2: not found\n");
#endif
        return false;
    }

#ifdef DEBUG
    OutputDebugStringW(L"@MilkFeng apply_rule_inner 4.0\n");
#endif

    // calculate relative path
    std::wstring abs_new_path = absolute(path_).wstring();
    std::wstring abs_rule_src = absolute(src_).wstring();
    int start = abs_new_path.find(abs_rule_src);
    Path relative = abs_new_path.substr(start + abs_rule_src.size());

    // delete the first '\'
    String relative_str = relative.wstring();
    while (!relative_str.empty() && relative_str.front() == L'\\') {
        relative_str = relative_str.substr(1);
    }

    if (relative_str.empty()) {
        result = dst_;
        return true;
    } else {
        relative = relative_str;
    }
#ifdef DEBUG
    debug_info = L"@MilkFeng apply_rule_inner 4: "s + relative.wstring() + L" " + path_.wstring() + L" " + src_.wstring() + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    // concat relative path with dst
    result = dst_ / relative;

#ifdef DEBUG
    debug_info = L"@MilkFeng apply_rule_inner 4 5: "s + result.wstring() + L"\n"s;
    OutputDebugStringW(debug_info.c_str());
#endif

    return true;
}

static bool apply_rule_inner (const Env &env, const Path &path, const Rule &rule, Path &result) {
    // translate rule
    TranslatedRule tr = translate_rule(env, rule);

    // apply rule
    return apply_rule_inner(path, tr, result);
}

void Env::add_rule (const Rule &rule) {
    this->rules.push_back(rule);
}

bool Env::apply_rules (const Path &path, Path &result) const {
    for (const Rule &rule: this->rules) {
        if (apply_rule_inner(*this, path, rule, result)) {
            return true;
        }
    }
    return false;
}

Path Env::env_path () {
    // AppData\Roaming\com.milkfeng.galhub\env.json
    Path app_data = SystemFolders::default_system_folders().roaming_app_data;
    Path env_folder = app_data / L"com.milkfeng.galhub";
    return env_folder / ENV_FILE_NAME;
}

Env Env::read_env () {
    Path file_path = env_path();
    std::wfstream file(file_path, std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    std::wstring s = read_wstring_from_file(file_path);
    return Json::parse(s);
}

void Env::write_env (const Env &env) {
    Path filePath = env_path();
    if (!std::filesystem::exists(filePath.parent_path())) {
        std::filesystem::create_directories(filePath.parent_path());
    }

    std::wstring s = Json::dump(env);
    write_wstring_to_file(filePath, s);
}

const std::vector<String> &Env::folder_vars () {
    static bool initialized = false;
    static std::vector<String> vars;

    if (!initialized) {
        std::vector<String> system_vars = SystemFolders::vars();
        std::vector<String> env_vars = EnvFolders::vars();

        vars.reserve(1 + system_vars.size() + env_vars.size());
        vars.insert(vars.end(), system_vars.begin(), system_vars.end());
        vars.insert(vars.end(), env_vars.begin(), env_vars.end());

        initialized = true;
    }
    return vars;
}

Path Env::folder_var_to_path (const String &var) const {
    const auto &system_vars = SystemFolders::vars();
    const auto &env_vars = EnvFolders::vars();

    if (std::find(system_vars.begin(), system_vars.end(), var) != system_vars.end()) {
        return system_folders.to_map()[var];
    } else if (std::find(env_vars.begin(), env_vars.end(), var) != env_vars.end()) {
        return env_folders.to_map()[var];
    } else {
        throw std::runtime_error("Unknown base path");
    }
}
