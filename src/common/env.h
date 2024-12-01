#ifndef PATH_TRANSLATION_H
#define PATH_TRANSLATION_H

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>

#include "json_extend.hpp"

using Path = std::filesystem::path;
using String = std::wstring;

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


    CONFIGOR_BIND(Json::value, SystemFolders, REQUIRED_WIDE(user_profile),
                  REQUIRED_WIDE(desktop), REQUIRED_WIDE(documents), REQUIRED_WIDE(downloads), REQUIRED_WIDE(music),
                  REQUIRED_WIDE(pictures), REQUIRED_WIDE(videos), REQUIRED_WIDE(saved_games),
                  REQUIRED_WIDE(app_data), REQUIRED_WIDE(local_app_data), REQUIRED_WIDE(local_low_app_data));

    static SystemFolders default_system_folders ();

    [[nodiscard]] std::map<String, Path> to_map () const;
    static std::set<String> vars ();
};

struct EnvFolders {
    Path env_folder;            // <gal hub root dir>\.env
    Path game_folder;           // <gal hub root dir>\.env\<game name>

    CONFIGOR_BIND(Json::value, EnvFolders, REQUIRED_WIDE(env_folder), REQUIRED_WIDE(game_folder));

    [[nodiscard]] std::map<String, Path> to_map () const;
    static std::set<String> vars ();
};

struct Rule {
    String src_base;
    Path src;

    String dst_name;

    CONFIGOR_BIND(Json::value, Rule, REQUIRED_WIDE(src_base), REQUIRED_WIDE(src), REQUIRED_WIDE(dst_name));
};

struct Env {
    static constexpr auto ENV_FILE_NAME = L"env.json";

    SystemFolders system_folders;
    EnvFolders env_folders;

    std::vector<Rule> rules;

    CONFIGOR_BIND(Json::value, Env, REQUIRED_WIDE(system_folders), REQUIRED_WIDE(env_folders), REQUIRED_WIDE(rules));

    void add_rule (const Rule &rule);

    [[nodiscard]] Path apply_rules (const Path &path) const;

    static Path env_path ();
    static Env read_env ();
    static void write_env (const Env &env);

    static std::set<String> folder_vars ();
};


#endif //PATH_TRANSLATION_H
