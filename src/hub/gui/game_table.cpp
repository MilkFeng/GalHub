#include "game_table.h"

#include <QMouseEvent>
#include <QHeaderView>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>

#include "auto_deselect_event_filter.h"
#include "game_config_dialog.h"
#include "../runner.h"


void GameTableWidget::clear_table () {
    setRowCount(0);
}

void GameTableWidget::add_game_item (const GameItem &game_item) {
    int row = rowCount();
    insertRow(row);

    auto thumbnail = game_item.thumbnail.scaled(64, 64);
    auto *game_name = new QTableWidgetItem(game_item.name);
    auto *game_exe_path = new QTableWidgetItem(game_item.path.path());

    QString rule_string;
    for (int i = 0; i < game_item.rules.size(); i++) {
        const auto &rule = game_item.rules[i];

        if (!rule.src_base.empty()) {
            rule_string += QString::number(i + 1) + ". %" + QString::fromStdWString(rule.src_base) + "%/"
                + QString::fromStdWString(rule.src) + " → "
                + QString::fromStdWString(rule.dst_name);
        } else {
            rule_string += QString::number(i + 1) + ". "
                + QString::fromStdWString(rule.src) + " → "
                + QString::fromStdWString(rule.dst_name);
        }

        if (i != game_item.rules.size() - 1) {
            rule_string += "\n";
        }
    }

    if (rule_string.isEmpty()) {
        rule_string = "无";
    }

    auto *game_rules = new QTableWidgetItem(rule_string);

    QIcon icon(thumbnail);
    game_name->setIcon(icon);

    setItem(row, 0, game_name);
    setItem(row, 1, game_exe_path);
    setItem(row, 2, game_rules);
}

void GameTableWidget::show_context_menu (const QPoint &pos, int row) {
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

void GameTableWidget::sort_by_column (int column) {
    // 排序
    sortItems(column, horizontalHeader()->sortIndicatorOrder());
}

GameTableWidget::GameTableWidget (QWidget *parent): QTableWidget(parent) {
    setColumnCount(3);
    setIconSize(QSize(64, 128));
    setHorizontalHeaderLabels(QStringList() << "游戏名称" << "游戏路径" << "重定向规则");

    // 不显示左侧序号
    verticalHeader()->setVisible(false);

    // 去掉四边的边框和分割线
    setFrameShape(QFrame::NoFrame);
    setShowGrid(false);

    // 选择时选择一行，并且只能选择一行
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    // 表头最后一列填充整个空间
    horizontalHeader()->setStretchLastSection(true);

    // 滑动时平滑滑动
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 关闭修改
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 排序
    horizontalHeader()->setSortIndicatorShown(true);
    horizontalHeader()->setSectionsClickable(true);
    horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
    connect(horizontalHeader(), &QHeaderView::sectionClicked, this, &GameTableWidget::sort_by_column);

    // 自动更新表格
    connect(&EnvManager::instance(), &EnvManager::config_changed, this, &GameTableWidget::on_config_upd);

    // 自动取消选中
    auto *event_filter = new AutoDeselectEventFilter(this);
    viewport()->installEventFilter(event_filter);

    // 右键菜单
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &GameTableWidget::on_right_click);

    // 双击事件
    connect(this, &QTableWidget::cellDoubleClicked, this, &GameTableWidget::on_double_click);

    on_config_upd();
}

void GameTableWidget::on_config_upd () {
    const Config &config = EnvManager::instance().config();

    // TODO: 更高效的更新表格
    clear_table();

    for (const auto &[_, game_config]: config.game_configs) {
        // 读取缩略图
        QPixmap thumbnail(QString::fromStdWString(game_config.thumbnail_path));
        QString game_name = QString::fromStdWString(game_config.game_name);
        QDir game_exe_path = game_config.original_game_path;
        QList<Rule> rules;
        for (const auto &rule: game_config.rules) {
            rules.append(rule);
        }

        add_game_item({thumbnail, game_name, game_exe_path, rules});
    }

    // 调整列宽
    resizeColumnsToContents();

    // 调整行高
    verticalHeader()->setDefaultSectionSize(60);

    // 取消选中
    setCurrentItem(nullptr);

    // 排序
    int sort_column = horizontalHeader()->sortIndicatorSection();
    sort_by_column(sort_column);

    update();
}

void GameTableWidget::on_right_click (const QPoint &pos) {
    QModelIndex index = indexAt(pos);
    qDebug() << "right click at" << index.row();
    if (index.isValid()) {
        int row = index.row();
        show_context_menu(pos, row);
    }
}

void GameTableWidget::on_double_click (int row, int column) const {
    // 获取游戏名称
    auto *game_name_item = item(row, 0);
    QString game_name = game_name_item->text();

    // 启动
    Runner::run(game_name.toStdWString());
}

void GameTableWidget::on_edit_action_triggered (int row) const {
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

void GameTableWidget::on_delete_action_triggered (int row) const {
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