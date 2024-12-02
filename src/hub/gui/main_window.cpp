#include "main_window.h"

#include <QMenuBar>

#include "game_config_dialog.h"
#include "game_table.h"

MainWindow::MainWindow (QWidget *parent) {

    resize(800, 600);

    // 添加一个菜单
    auto *menuBar = new QMenuBar(this);
    auto *fileMenu = menuBar->addMenu("文件");
    auto *addGameAction = fileMenu->addAction("添加游戏");
    addGameAction->setShortcut(QKeySequence("Ctrl+A"));
    connect(addGameAction, &QAction::triggered, this, &MainWindow::on_add_game_action_triggered);
    setMenuBar(menuBar);

    // 添加一个表格
    auto *gameTable = new GameTableWidget(this);
    setCentralWidget(gameTable);
}

void MainWindow::on_add_game_action_triggered () {
    GameConfigDialog dialog(this);
    auto result = dialog.exec();

    if (result == QDialog::Accepted) {
        auto gameConfig = dialog.game_config();
        EnvManager::instance().init_env_of_game(gameConfig);

        auto config = EnvManager::instance().config();
        config.upsert_game(gameConfig);
        EnvManager::instance().upd_config(config);

        EnvManager::instance().write_config();
    }
}
