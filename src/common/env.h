#ifndef PATH_TRANSLATION_H
#define PATH_TRANSLATION_H

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>

#include <json.hpp>

using Path = std::filesystem::path;
using String = std::wstring;
using Json = configor::wjson;

struct SystemFolders {
    // User Folders =============================================================

    Path user_profile;          // C:\Users\<username>

    Path desktop;               // default is C:\Users\<username>\Desktop but can be changed to another location
    Path documents;             // default is C:\Users\<username>\Documents but can be changed to another location
    Path downloads;             // default is C:\Users\<username>\Downloads but can be changed to another location
    Path music;                 // default is C:\Users\<username>\Music but can be changed to another location
    Path pictures;              // default is C:\Users\<username>\Pictures but can be changed to another location
    Path videos;                // default is C:\Users\<username>\Videos but can be changed to another location
    Path saved_games;           // default is C:\Users\<username>\Saved Games but can be changed to another location

    // Program Folders ==========================================================

    Path app_data;              // C:\Users\<username>\AppData\Roaming
    Path local_app_data;        // C:\Users\<username>\AppData\Local
    Path local_low_app_data;    // C:\Users\<username>\AppData\LocalLow

    CONFIGOR_BIND(Json::value, SystemFolders, REQUIRED(user_profile, L"user_profile"),
                  REQUIRED(desktop, L"desktop"), REQUIRED(documents, L"documents"), REQUIRED(downloads, L"downloads"), REQUIRED(music, L"music"),
                  REQUIRED(pictures, L"pictures"), REQUIRED(videos, L"videos"), REQUIRED(saved_games, L"saved_games"),
                  REQUIRED(app_data, L"app_data"), REQUIRED(local_app_data, L"local_app_data"), REQUIRED(local_low_app_data, L"local_low_app_data"));

    static SystemFolders default_system_folders ();

    [[nodiscard]] std::map<String, Path> to_map () const;
};

struct EnvFolders {
    Path env_folder;            // <gal hub root dir>\.env
    Path game_folder;           // <gal hub root dir>\.env\<game name>

    CONFIGOR_BIND(Json::value, EnvFolders, REQUIRED(env_folder, L"env_folder"), REQUIRED(game_folder, L"game_folder"));

    [[nodiscard]] std::map<String, Path> to_map () const;
};

struct Rule {
    String src_base;
    Path src;

    String dst_name;

    CONFIGOR_BIND(Json::value, Rule, REQUIRED(src_base, L"src_base"), REQUIRED(src, L"src"), REQUIRED(dst_name, L"dst_name"));
};

struct Env {
    static constexpr auto ENV_FILE_NAME = L"env.json";

    SystemFolders system_folders;
    EnvFolders env_folders;

    std::vector<Rule> rules;

    CONFIGOR_BIND(Json::value, Env, REQUIRED(system_folders, L"system_folders"), REQUIRED(env_folders, L"env_folders"), REQUIRED(rules, L"rules"));

    void add_rule (const Rule &rule);

    [[nodiscard]] Path apply_rules (const Path &path) const;

    static Path env_path ();
    static Env read_env ();
    static void write_env (const Env &env);
};


#endif //PATH_TRANSLATION_H
