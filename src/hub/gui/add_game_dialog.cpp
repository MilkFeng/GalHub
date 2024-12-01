#include "add_game_dialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AddGameDialog::AddGameDialog (QWidget *parent) : QDialog(parent) {
    setWindowTitle("添加游戏");

    auto *layout = new QGridLayout(this);
    setLayout(layout);

    layout->addWidget(new QLabel("游戏名称"), 0, 0);
    layout->addWidget(new QLineEdit(), 0, 1);

    layout->addWidget(new QLabel("游戏缩略图"), 2, 0);
    layout->addWidget(new QLineEdit(), 2, 1);
    layout->addWidget(new QPushButton("选择"), 2, 2);

    layout->addWidget(new QLabel("游戏目录"), 1, 0);
    layout->addWidget(new QLineEdit(), 1, 1);
    layout->addWidget(new QPushButton("选择"), 1, 2);

}
