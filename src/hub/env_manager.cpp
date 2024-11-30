#include "env_manager.h"


#include <fstream>
#include <iostream>
#include <minwindef.h>
#include <windows.h>

Config Config::default_config () {
    Config config;
    config.env_folder_name = L"Env";
    return config;
}

EnvManager &EnvManager::instance () {
    static EnvManager instance;
    return instance;
}

void EnvManager::init_env () const {
    // create essential folders
    Path env_folder_ = env_folder();
    if (!std::filesystem::exists(env_folder_)) {
        std::filesystem::create_directory(env_folder_);
    }

    for (const auto &[game_name, game_config]: config().game_configs) {
        Path game_folder = env_folder_ / game_name;
        if (!std::filesystem::exists(game_folder)) {
            std::filesystem::create_directory(game_folder);
        }

        for (const auto &rule: game_config.rules) {
            Path src_folder = game_folder / rule.dst_name;
            if (!std::filesystem::exists(src_folder)) {
                std::filesystem::create_directory(src_folder);
            }
        }
    }
}

const Config & EnvManager::config () const {
    return _config;
}

static Path working_dir () {
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, path, MAX_PATH)) {
        throw std::runtime_error("Failed to get module file name");
    }

    return std::filesystem::absolute(path).parent_path();
}

Path EnvManager::config_path () {
    return working_dir() / CONFIG_FILE_NAME;
}

Path EnvManager::env_folder () const {
    return working_dir() / config().env_folder_name;
}

void EnvManager::read_config () {
    Path config_file_path = config_path();

    if (!std::filesystem::exists(config_file_path)) {
        // if config file does not exist, create one
        _config = Config::default_config();
        write_config();
        return;
    }

    std::wfstream file(config_file_path, std::ios::in);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    String s, t;
    while (std::getline(file, t)) {
        s += t + L"\n";
    }
    _config = Json::parse(s);
}

void EnvManager::write_config () const {
    Path config_file_path = config_path();
    wprintf(L"write %s\n", config_file_path.c_str());

    std::wfstream file(config_file_path, std::ios::out);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    file << Json::wrap(_config);
}

void EnvManager::gen_env_for_game (const String &game_name) const {
    Env env;

    env.system_folders = SystemFolders::default_system_folders();

    Path env_folder_ = env_folder();
    env.env_folders = {
        env_folder_,
        env_folder_ / game_name
    };

    env.rules = config().game_configs.at(game_name).rules;

    // write env to file
    Env::write_env(env);
}
