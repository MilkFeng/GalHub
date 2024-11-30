#ifndef ENV_MANAGER_H
#define ENV_MANAGER_H

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../common/env.h"

struct GameConfig {
    Path original_game_path;        // game exe path
    String game_name;               // game name

    std::vector<Rule> rules;        // rules

    CONFIGOR_BIND(Json::value, GameConfig, REQUIRED(original_game_path, L"original_game_path"), REQUIRED(game_name, L"game_name"), REQUIRED(rules, L"rules"));
};

struct Config {
    String env_folder_name;
    std::map<String, GameConfig> game_configs;

    CONFIGOR_BIND(Json::value, Config, REQUIRED(env_folder_name, L"env_folder_name"), REQUIRED(game_configs, L"game_configs"));

    static Config default_config ();
};

class EnvManager {
    Config _config;
    static constexpr auto CONFIG_FILE_NAME = L"config.json";

    EnvManager () = default;

public:
    static EnvManager &instance();

    void init_env () const;

    [[nodiscard]] const Config &config() const;

    static Path config_path ();
    [[nodiscard]] Path env_folder () const;

    void read_config ();
    void write_config () const;

    void gen_env_for_game (const String &game_name) const;
};

#endif //ENV_MANAGER_H
