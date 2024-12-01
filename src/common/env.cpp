#include "env.h"

#include <fstream>
#include <map>
#include <windows.h>
#include <PathCch.h>
#include <shlobj.h>
#include <vector>

static Path get_env_var (const wchar_t *name) {
    wchar_t *path;
    _wdupenv_s(&path, nullptr, name);
    std::wstring result = path;
    free(path);
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

    folders.app_data = get_folder_path(FOLDERID_RoamingAppData);
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

        {L"APP_DATA", app_data},
        {L"LOCAL_APP_DATA", local_app_data},
        {L"LOCAL_LOW_APP_DATA", local_low_app_data}
    };
}

std::set<String> SystemFolders::vars () {
    return {
        L"USER_PROFILE",

        L"DESKTOP",
        L"DOCUMENTS",
        L"DOWNLOADS",
        L"MUSIC",
        L"PICTURES",
        L"VIDEOS",
        L"SAVED_GAMES",

        L"APP_DATA",
        L"LOCAL_APP_DATA",
        L"LOCAL_LOW_APP_DATA"
    };
}

std::map<String, Path> EnvFolders::to_map () const {
    return {
        {L"ENV", env_folder},
        {L"GAME", game_folder}
    };
}

std::set<String> EnvFolders::vars () {
    return {
        L"ENV",
        L"GAME"
    };
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

    Path dst = env.env_folders.game_folder / rule.dst_name;

    return TranslatedRule{src, dst};
}


static bool apply_rule_inner (const Path &path, const TranslatedRule &rule, Path &result) {
    // if path is not in src, return false
    if (absolute(path).wstring().find(absolute(rule.src).wstring()) == std::string::npos) {
        return false;
    }

    // calculate relative path
    Path relative = path.lexically_relative(rule.src);

    // concat relative path with dst
    result = rule.dst / relative;
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

Path Env::apply_rules (const Path &path) const {
    for (const Rule &rule: this->rules) {
        Path result;
        if (apply_rule_inner(*this, path, rule, result)) {
            return result;
        }
    }
    return path;
}

Path Env::env_path () {
    // AppData\Roaming\com.milkfeng.galhub\env.json
    Path app_data = SystemFolders::default_system_folders().app_data;
    Path env_folder = app_data / L"com.milkfeng.galhub";
    return env_folder / ENV_FILE_NAME;
}

Env Env::read_env () {
    Path filePath = env_path();
    std::wfstream file(filePath, std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    String s, t;
    while (std::getline(file, t)) {
        s += t + L"\n";
    }
    return Json::parse(s);
}

void Env::write_env (const Env &env) {
    Path filePath = env_path();
    if (!std::filesystem::exists(filePath.parent_path())) {
        std::filesystem::create_directories(filePath.parent_path());
    }
    std::wfstream file(filePath, std::ios::out);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    file << Json::wrap(env);
}

std::set<String> Env::folder_vars () {
    std::set<String> result = SystemFolders::vars();
    result.merge(EnvFolders::vars());
    return result;
}
