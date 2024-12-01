#include "env_manager.h"


#include <fstream>
#include <iostream>
#include <utility>
#include <minwindef.h>
#include <windows.h>

GameConfig::GameConfig (Path thumbnail_path, Path original_game_path, String game_name, std::vector<Rule> rules)
    : thumbnail_path(std::move(thumbnail_path)), original_game_path(std::move(original_game_path)),
      game_name(std::move(game_name)), rules(std::move(rules)) {
}

GameConfig::GameConfig (Path thumbnail_path, Path original_game_path, String game_name)
    : thumbnail_path(std::move(thumbnail_path)), original_game_path(std::move(original_game_path)),
      game_name(std::move(game_name)) {
}

void GameConfig::add_rule (const Rule &rule) {
    rules.push_back(rule);
}

bool GameConfig::check_meta_info (String &message) const {
    if (original_game_path.empty() || game_name.empty()) {
        message = L"游戏路径和游戏名不能为空";
        return false;
    }

    if (original_game_path.extension() != L".exe") {
        message = L"游戏路径必须是exe文件";
        return false;
    }

    if (!std::filesystem::exists(original_game_path)) {
        message = L"游戏路径不存在";
        return false;
    }

    if (!thumbnail_path.empty() && !std::filesystem::exists(thumbnail_path)) {
        message = L"缩略图路径不存在";
        return false;
    }
    return true;
}

bool GameConfig::check_rules (String &message) const {
    return true;
}

bool GameConfig::check (String &message) const {
    if (!check_meta_info(message)) {
        return false;
    }

    if (!check_rules(message)) {
        return false;
    }

    return true;
}


Config Config::default_config () {
    Config config;
    config.env_folder_name = L"Env";
    return config;
}

bool Config::has_game (const String &game_name) const {
    return game_configs.find(game_name) != game_configs.end();
}

void Config::upsert_game (const GameConfig &game_config) {
    game_configs[game_config.game_name] = game_config;
}

void Config::del_game (const String &game_name) {
    game_configs.erase(game_name);
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

    for (const auto &[_, game_config]: config().game_configs) {
        init_env_of_game(game_config);
    }
}

void EnvManager::init_env_of_game (const String &game_name) const {
    init_env_of_game(config().game_configs.at(game_name));
}

void EnvManager::init_env_of_game (const GameConfig &game_config) const {
    Path env_folder_ = env_folder();
    if (!std::filesystem::exists(env_folder_)) {
        std::filesystem::create_directory(env_folder_);
    }
    Path game_folder = env_folder_ / game_config.game_name;
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

const Config &EnvManager::config () const {
    return _config;
}

void EnvManager::upd_config (const Config &config) {
    _config = config;
    emit config_changed();
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
        upd_config(Config::default_config());
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

    upd_config(Json::parse(s));
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
