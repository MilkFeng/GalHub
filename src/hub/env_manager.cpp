#include "env_manager.h"


#include <fstream>
#include <iostream>
#include <utility>

#include "../common/util.h"

GameConfig::GameConfig (Path thumbnail_path, Path original_game_path, String game_name, std::vector<Rule> rules)
    : thumbnail_path(std::move(thumbnail_path)), original_game_path(std::move(original_game_path)),
      game_name(std::move(game_name)), rules(std::move(rules)) {
}

GameConfig::GameConfig (Path thumbnail_path, Path original_game_path, String game_name)
    : thumbnail_path(std::move(thumbnail_path)), original_game_path(std::move(original_game_path)),
      game_name(std::move(game_name)) {
}

Path GameConfig::thumbnail_full_path () const {
    return std::filesystem::absolute(working_dir() / thumbnail_path);
}

Path GameConfig::original_game_full_path () const {
    return std::filesystem::absolute(working_dir() / original_game_path);
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
    if (std::any_of(rules.begin(), rules.end(), [](const Rule &rule) {
        return rule.src.empty() || rule.dst_name.empty();
    })) {
        message = L"规则的源相对路径和目标文件夹名称不能为空";
        return false;
    }

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

Path EnvManager::config_path () {
    return working_dir() / CONFIG_FILE_NAME;
}

Path EnvManager::env_folder () const {
    return (working_dir() / config().env_folder_name).wstring() + L"\\";
}

void EnvManager::read_config () {
    Path config_file_path = config_path();

    if (!std::filesystem::exists(config_file_path)) {
        // if config file does not exist, create one
        upd_config(Config::default_config());
        write_config();
        return;
    }

    String s = read_wstring_from_file(config_file_path);

    try {
        upd_config(Json::parse(s));
    } catch (const std::exception &e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        upd_config(Config::default_config());
        write_config();
    }
}

void EnvManager::write_config () const {
    Path config_file_path = config_path();
    std::wcout << "Writing config to " << config_file_path << std::endl;

    std::wstring json_str = Json::dump(_config);
    write_wstring_to_file(config_file_path, json_str);
}

void EnvManager::gen_env_for_game (const String &game_name) const {
    Env env = get_env_for_game(game_name);

    // write env to file
    Env::write_env(env);
}

Env EnvManager::get_env_for_game (const String &game_name) const {
    Env env;

    auto game_config = config().game_configs.at(game_name);

    env.system_folders = SystemFolders::default_system_folders();

    Path env_folder_ = env_folder();
    Path game_folder_ = (env_folder_ / game_name).wstring() + L"\\";
    Path original_game_folder_ = game_config.original_game_full_path().parent_path().wstring() + L"\\";

    env.env_folders = {
        env_folder_,
        game_folder_,
        original_game_folder_
    };

    env.rules = game_config.rules;
    return env;
}

const std::vector<String> &EnvManager::folder_vars () {
    return Env::folder_vars();
}

const String DEFAULT_VAR = L"GAME";
const String &EnvManager::default_var () {
    return DEFAULT_VAR;
}

Path EnvManager::folder_var_to_path (const String &var, const String &game_name) const {
    Env env;

    env.system_folders = SystemFolders::default_system_folders();

    Path env_folder_ = env_folder();
    Path game_folder_ = (env_folder_ / game_name).wstring() + L"\\";
    env.env_folders = {
        env_folder_,
        game_folder_,
    };

    return env.folder_var_to_path(var);
}
