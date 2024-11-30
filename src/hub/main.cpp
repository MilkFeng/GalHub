#include <iostream>

#include "env_manager.h"
#include "runner.h"

int main () {
    EnvManager env_manager = EnvManager::instance();

    try {
        env_manager.read_config();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    env_manager.init_env();

    auto config = env_manager.config();
    String json = Json::dump(config);

    std::wcout << json << std::endl;

    env_manager.read_config();
    std::cout << "read config finish" << std::endl;
    env_manager.gen_env_for_game(L"white album 2");
    auto game_config = config.game_configs[L"white album 2"];

    Path exe_path = game_config.original_game_path;

    std::wcout << Env::env_path() << std::endl;
    Env env = Env::read_env();
    std::wcout << Json::wrap(env) << std::endl;

    for (auto &rule: env.rules) {
        std::wcout << rule.src_base << L' ' << rule.src << L' ' << rule.dst_name << std::endl;
    }

    Path e2 = env.apply_rules(L"D:\\Windows\\Documents\\Leaf\\WHITE ALBUM2\\D3D.ini");
    std::wcout << e2 << std::endl;

    Runner::run(L"white album 2");
    return 0;
}
