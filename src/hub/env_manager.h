#ifndef ENV_MANAGER_H
#define ENV_MANAGER_H

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <QObject>
#include <windows.h>

#include "../common/env.h"

struct GameConfig {
    Path thumbnail_path;            // thumbnail path relative to the hub root dir
    Path original_game_path;        // game exe path relative to the hub root dir

    String game_name;               // game name

    std::vector<Rule> rules;        // rules

    CONFIGOR_BIND(Json::value, GameConfig, REQUIRED_WIDE(thumbnail_path), REQUIRED_WIDE(original_game_path),
                  REQUIRED_WIDE(game_name), REQUIRED_WIDE(rules));

    GameConfig () = default;
    GameConfig (Path thumbnail_path, Path original_game_path, String game_name, std::vector<Rule> rules);
    GameConfig (Path thumbnail_path, Path original_game_path, String game_name);

    [[nodiscard]] Path thumbnail_full_path () const;
    [[nodiscard]] Path original_game_full_path () const;

    void add_rule (const Rule &rule);

    bool check_meta_info (String &message) const;
    bool check_rules (String &message) const;
    bool check (String &message) const;
};

struct Config {
    String env_folder_name;
    std::map<String, GameConfig> game_configs;

    CONFIGOR_BIND(Json::value, Config, REQUIRED_WIDE(env_folder_name), REQUIRED_WIDE(game_configs));

    static Config default_config ();

    [[nodiscard]] bool has_game (const String &game_name) const;
    void upsert_game (const GameConfig &game_config);
    void del_game (const String &game_name);
};

class EnvManager final : public QObject {
    Q_OBJECT

    Config _config;
    static constexpr auto CONFIG_FILE_NAME = L"config.json";

    EnvManager () = default;
public:
    EnvManager (const EnvManager &) = delete;
    EnvManager &operator= (const EnvManager &) = delete;

    static EnvManager &instance();

    void init_env () const;
    void init_env_of_game (const String &game_name) const;
    void init_env_of_game (const GameConfig &game_config) const;

    [[nodiscard]] const Config &config() const;
    void upd_config (const Config &config);

    static Path config_path ();
    [[nodiscard]] Path env_folder () const;

    void read_config ();
    void write_config () const;

    void gen_env_for_game (const String &game_name) const;
    [[nodiscard]] Env get_env_for_game (const String &game_name) const;

    static const std::vector<String> &folder_vars ();
    static const String &default_var ();
    [[nodiscard]] Path folder_var_to_path (const String &var, const String &game_name) const;

signals:
    void config_changed ();

};

#endif //ENV_MANAGER_H
