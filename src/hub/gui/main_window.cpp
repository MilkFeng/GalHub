#include "main_window.h"

#include <QMenuBar>

#include "add_game_dialog.h"
#include "game_table.h"

MainWindow::MainWindow (QWidget *parent) {
    // 添加一个菜单
    auto *menuBar = new QMenuBar(this);
    auto *fileMenu = menuBar->addMenu("文件");
    auto *addGameAction = fileMenu->addAction("添加游戏");
    addGameAction->setShortcut(QKeySequence("Ctrl+A"));
    connect(addGameAction, &QAction::triggered, this, [this] {
        AddGameDialog dialog(this);
        dialog.exec();
    });

    setMenuBar(menuBar);

    // 添加一个表格
    auto *gameTable = new GameTable(this);
    setCentralWidget(gameTable);
}
