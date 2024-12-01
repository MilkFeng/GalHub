#include "game_table.h"

#include <QMouseEvent>
#include <QHeaderView>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>

#include "game_config_dialog.h"
#include "../runner.h"


void GameTable::clear_table () {
    setRowCount(0);
}

void GameTable::add_game_item (const GameItem &game_item) {
    int row = rowCount();
    insertRow(row);

    auto thumbnail = game_item.thumbnail.scaled(64, 64);
    auto *game_name = new QTableWidgetItem(game_item.name);
    auto *game_exe_path = new QTableWidgetItem(game_item.path.absolutePath());

    QIcon icon(thumbnail);
    game_name->setIcon(icon);

    setItem(row, 0, game_name);
    setItem(row, 1, game_exe_path);
}

void GameTable::show_context_menu (const QPoint &pos, int row) {
    QMenu menu;

    // 添加菜单项
    auto *edit_action = new QAction("编辑", this);
    connect(edit_action, &QAction::triggered, [this, row] { on_edit_action_triggered(row); });

    auto *delete_action = new QAction("删除", this);
    connect(delete_action, &QAction::triggered, [this, row] { on_delete_action_triggered(row); });

    menu.addAction(edit_action);
    menu.addAction(delete_action);

    menu.exec(viewport()->mapToGlobal(pos));
}

GameTable::GameTable (QWidget *parent): QTableWidget(parent) {
    setColumnCount(2);
    setIconSize(QSize(64, 128));
    setHorizontalHeaderLabels(QStringList() << "游戏名称" << "游戏路径");

    // 不显示左侧序号
    verticalHeader()->setVisible(false);

    // 选择时选择一行，并且只能选择一行
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    // 表头最后一列填充整个空间
    horizontalHeader()->setStretchLastSection(true);

    // 滑动时平滑滑动
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 关闭修改
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 自动更新表格
    connect(&EnvManager::instance(), &EnvManager::config_changed, this, &GameTable::on_config_upd);
    on_config_upd();

    // 点击事件
    auto *event_filter = new GameTableEventFilter(this);
    viewport()->installEventFilter(event_filter);

    // 右键菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &GameTable::on_right_click);

    // 双击事件
    connect(this, &QTableWidget::cellDoubleClicked, this, &GameTable::on_double_click);
}

void GameTable::on_config_upd () {
    const Config &config = EnvManager::instance().config();

    // TODO: 更高效的更新表格
    clear_table();

    for (const auto &[_, game_config]: config.game_configs) {
        // 读取缩略图
        QPixmap thumbnail(QString::fromStdWString(game_config.thumbnail_path));
        QString game_name = QString::fromStdWString(game_config.game_name);
        QDir game_exe_path = game_config.original_game_path;

        add_game_item({thumbnail, game_name, game_exe_path});
    }

    // 调整列宽
    resizeColumnsToContents();

    // 调整行高
    verticalHeader()->setDefaultSectionSize(128);
}

void GameTable::on_right_click (const QPoint &pos) {
    QModelIndex index = indexAt(pos);
    qDebug() << "right click at" << index.row();
    if (index.isValid()) {
        int row = index.row();
        show_context_menu(pos, row);
    }
}

void GameTable::on_double_click (int row, int column) const {
    // 获取游戏名称
    auto *game_name_item = item(row, 0);
    QString game_name = game_name_item->text();

    // 启动
    Runner::run(game_name.toStdWString());
}

void GameTable::on_edit_action_triggered (int row) const {
    // 获取游戏名称
    auto *game_name_item = item(row, 0);
    QString game_name = game_name_item->text();

    // 获取游戏配置
    const Config &config = EnvManager::instance().config();
    const GameConfig &game_config = config.game_configs.at(game_name.toStdWString());

    // 打开编辑对话框
    GameConfigDialog dialog(game_config);

    // 更新配置
    auto result = dialog.exec();

    if (result == QDialog::Accepted) {
        auto gameConfig = dialog.game_config();
        EnvManager::instance().init_env_of_game(gameConfig);

        auto config = EnvManager::instance().config();

        // 删除旧的配置
        config.del_game(game_name.toStdWString());

        // 添加新的配置
        config.upsert_game(gameConfig);

        // 更新配置
        EnvManager::instance().upd_config(config);
        EnvManager::instance().write_config();
    }
}

void GameTable::on_delete_action_triggered (int row) const {
    // 获取游戏名称
    auto *game_name_item = item(row, 0);
    QString game_name = game_name_item->text();

    // 询问是否删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        nullptr, "警告", "是否删除游戏 " + game_name + "？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::No) return;

    // 删除配置
    auto config = EnvManager::instance().config();
    config.del_game(game_name.toStdWString());

    // 更新配置
    EnvManager::instance().upd_config(config);
    EnvManager::instance().write_config();
}

void GameTableEventFilter::deselect (QMouseEvent *e) const {
    // 获取当前鼠标坐标
    QPoint point = e->pos();

    // 根据鼠标坐标，获取此时鼠标按下时所在的行、列
    QModelIndex index = table->indexAt(point);

    // 判断该单元格是否是空单元格
    if (table->item(index.row(), index.column()) == nullptr) {
        //取消选中
        table->setCurrentItem(nullptr);
    }
}

bool GameTableEventFilter::eventFilter (QObject *watched, QEvent *event) {
    // 类型转换
    auto *e = dynamic_cast<QMouseEvent *>(event);
    // 判空
    if (e == nullptr) {
        return QObject::eventFilter(watched, event);
    }
    // 判断事件类型为鼠标点击
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
        // 点击空白处取消选中
        deselect(e);
    }
    return QObject::eventFilter(watched, event);
}

GameTableEventFilter::GameTableEventFilter (GameTable *table) : QObject(table), table(table) {
}
