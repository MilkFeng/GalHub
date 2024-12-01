#ifndef ENV_MANAGER_H
#define ENV_MANAGER_H

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <QObject>
#include <windows.h>

#include "../common/env.h"

struct GameConfig {
    Path original_game_path;        // game exe path
    String game_name;               // game name

    std::vector<Rule> rules;        // rules

    CONFIGOR_BIND(Json::value, GameConfig, REQUIRED_WIDE(original_game_path), REQUIRED_WIDE(game_name), REQUIRED_WIDE(rules));

    GameConfig () = default;
    GameConfig (Path original_game_path, String game_name, std::vector<Rule> rules);
    GameConfig (Path original_game_path, String game_name);

    void add_rule (const Rule &rule);
};

struct Config {
    String env_folder_name;
    std::map<String, GameConfig> game_configs;

    CONFIGOR_BIND(Json::value, Config, REQUIRED_WIDE(env_folder_name), REQUIRED_WIDE(game_configs));

    static Config default_config ();

    [[nodiscard]] bool has_game (const String &game_name) const;
    void upsert_game (const String &game_name, const GameConfig &game_config);
};

class EnvManager final : public QObject {
    Q_OBJECT

    Config _config;
    static constexpr auto CONFIG_FILE_NAME = L"config.json";

    EnvManager () = default;

public:
    static EnvManager &instance();

    void init_env () const;

    [[nodiscard]] const Config &config() const;
    void upd_config (const Config &config);

    static Path config_path ();
    [[nodiscard]] Path env_folder () const;

    void read_config ();
    void write_config () const;

    void gen_env_for_game (const String &game_name) const;

signals:
    void config_changed ();

};

#endif //ENV_MANAGER_H
