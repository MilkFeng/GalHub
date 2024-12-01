#include "game_table.h"

#include "../env_manager.h"

GameTable::GameTable (QWidget *parent): QTableWidget(parent) {
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList() << "名称" << "目录");

    connect(&EnvManager::instance(), &EnvManager::config_changed, this, &on_config_upd);
}


void GameTable::on_config_upd () {
    auto config = EnvManager::instance().config();


}