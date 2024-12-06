#include "game_config_rules_widget.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QListWidget>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QToolTip>

#include "auto_deselect_event_filter.h"
#include "path_edit_group.h"
#include "rule_diag.h"

void GameConfigRulesWidget::init_ui () {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // 上移、下移、添加、删除按钮
    auto *buttons = new QWidget(this);
    auto *buttons_layout = new QHBoxLayout(buttons);
    buttons_layout->setContentsMargins(0, 0, 0, 0);
    buttons->setLayout(buttons_layout);

    auto *help_button = new QPushButton("?");
    up_button = new QPushButton("∧");
    down_button = new QPushButton("∨");
    add_button = new QPushButton("+");
    del_button = new QPushButton("-");

    help_button->setFixedWidth(30);
    up_button->setFixedWidth(30);
    down_button->setFixedWidth(30);
    add_button->setFixedWidth(30);
    del_button->setFixedWidth(30);


    connect(help_button, &QPushButton::clicked, show_tool_tip);
    connect(up_button, &QPushButton::clicked, this, &GameConfigRulesWidget::move_up);
    connect(down_button, &QPushButton::clicked, this, &GameConfigRulesWidget::move_down);
    connect(add_button, &QPushButton::clicked, this, &GameConfigRulesWidget::add_rule);
    connect(del_button, &QPushButton::clicked, this, &GameConfigRulesWidget::del_rule);

    buttons_layout->addWidget(help_button);
    buttons_layout->addStretch();
    buttons_layout->addWidget(up_button);
    buttons_layout->addWidget(down_button);
    buttons_layout->addWidget(add_button);
    buttons_layout->addWidget(del_button);

    layout->addWidget(buttons);

    // 规则列表
    rules_table_widget = new QTableWidget(this);
    rules_table_widget->setColumnCount(3);
    rules_table_widget->setHorizontalHeaderLabels(QStringList() << "基文件夹" << "源文件夹相对路径" << "目标文件夹名称");

    // 不显示左侧序号
    rules_table_widget->verticalHeader()->setVisible(false);

    // 去掉四边的边框和分割线
    rules_table_widget->setFrameShape(QFrame::NoFrame);
    rules_table_widget->setShowGrid(false);

    // 选择时选择一行，并且只能选择一行
    rules_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    rules_table_widget->setSelectionMode(QAbstractItemView::SingleSelection);

    // 表头最后一列填充整个空间
    rules_table_widget->horizontalHeader()->setStretchLastSection(true);

    // 滑动时平滑滑动
    rules_table_widget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 禁止编辑
    rules_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 自动取消选中
    auto *event_filter = new AutoDeselectEventFilter(rules_table_widget);
    rules_table_widget->viewport()->installEventFilter(event_filter);

    // 双击事件
    connect(rules_table_widget, &QTableWidget::cellDoubleClicked, this, &GameConfigRulesWidget::on_double_click);

    // 选中事件
    connect(rules_table_widget, &QTableWidget::itemSelectionChanged, this, &GameConfigRulesWidget::on_table_item_selection_changed);

    // 表格数据更改事件
    connect(rules_table_widget, &QTableWidget::itemChanged, this, &GameConfigRulesWidget::on_table_item_changed);

    layout->addWidget(rules_table_widget);
    update_button_status();
}

void GameConfigRulesWidget::clear_table () const {
    rules_table_widget->setRowCount(0);
}

void GameConfigRulesWidget::load_rules (const QList<Rule> &rules) const {
    clear_table();
    for (const auto &rule: rules) {
        add_rule_to_end(rule);
    }

    // 自适应列宽
    rules_table_widget->resizeColumnsToContents();
    rules_table_widget->horizontalHeader()->setStretchLastSection(true);

    rules_table_widget->update();
}

void GameConfigRulesWidget::swap_row (int row1, int row2) const {
    int column_count = rules_table_widget->columnCount();
    for (int i = 0; i < column_count; ++i) {
        auto *item1 = rules_table_widget->takeItem(row1, i);
        auto *item2 = rules_table_widget->takeItem(row2, i);

        rules_table_widget->setItem(row1, i, item2);
        rules_table_widget->setItem(row2, i, item1);
    }
}

void GameConfigRulesWidget::add_rule_to (const Rule &rule, int row) const {
    // 向表格中添加一行
    auto *src_base_item = new QTableWidgetItem(QString::fromStdWString(rule.src_base));
    auto *src_item = new QTableWidgetItem(QString::fromStdWString(rule.src));
    auto *dst_name_item = new QTableWidgetItem(QString::fromStdWString(rule.dst_name));

    rules_table_widget->insertRow(row);
    rules_table_widget->setItem(row, 0, src_base_item);
    rules_table_widget->setItem(row, 1, src_item);
    rules_table_widget->setItem(row, 2, dst_name_item);
}

void GameConfigRulesWidget::add_rule_to_end (const Rule &rule) const {
    add_rule_to(rule, rules_table_widget->rowCount());
}

void GameConfigRulesWidget::move_up () const {
    // 获取当前选中的 item
    auto *current_item = rules_table_widget->currentItem();
    if (!current_item) return;

    // 获取当前选中的 item 的 index
    int current_row = rules_table_widget->row(current_item);
    if (current_row != 0) {
        // 交换两个 item 的位置
        swap_row(current_row, current_row - 1);
    }

    // 选中当前 item
    rules_table_widget->setCurrentItem(current_item);
}

void GameConfigRulesWidget::move_down () const {
    // 获取当前选中的 item
    auto *current_item = rules_table_widget->currentItem();
    if (!current_item) return;

    // 获取当前选中的 item 的 index
    int current_row = rules_table_widget->row(current_item);
    if (current_row != rules_table_widget->rowCount() - 1) {
        // 交换两个 item 的位置
        swap_row(current_row, current_row + 1);
    }

    // 选中当前 item
    rules_table_widget->setCurrentItem(current_item);
}

void GameConfigRulesWidget::add_rule () {
    // 创建一个新的 rule
    Rule rule;
    rule.src_base = EnvManager::default_var();
    int row;

    // 获取当前选中的 item
    auto *current_item = rules_table_widget->currentItem();
    if (current_item) {
        int current_row = rules_table_widget->row(current_item);
        add_rule_to(rule, current_row + 1);
        row = current_row + 1;
    } else {
        add_rule_to_end(rule);
        row = rules_table_widget->rowCount() - 1;
    }

    qDebug() << "add rule at" << row;
    qDebug() << "current row count" << rules_table_widget->rowCount();

    // 编辑
    if (!edit_rule(row)) {
        // 如果编辑失败，删除新添加的 item
        rules_table_widget->removeRow(row);
    }

    // 选中新的 item
    rules_table_widget->setCurrentIndex(rules_table_widget->model()->index(row, 0));
}

void GameConfigRulesWidget::del_rule () const {
    // 获取当前选中的 item
    auto *current_item = rules_table_widget->currentItem();
    if (!current_item) return;

    // 获取当前选中的 item 的 index
    int current_row = rules_table_widget->row(current_item);

    // 将选中的 item 一直 swap 到最后，然后删除最后一个 item
    for (int i = current_row; i < rules_table_widget->rowCount() - 1; ++i) {
        swap_row(i, i + 1);
    }
    rules_table_widget->removeRow(rules_table_widget->rowCount() - 1);

    if (current_row < rules_table_widget->rowCount()) {
        rules_table_widget->setCurrentIndex(rules_table_widget->model()->index(current_row, 0));
    } else if (rules_table_widget->rowCount() > 0) {
        rules_table_widget->setCurrentIndex(rules_table_widget->model()->index(rules_table_widget->rowCount() - 1, 0));
    }
}

void GameConfigRulesWidget::show_tool_tip () {
    QToolTip::showText(QCursor::pos(), "在打开游戏时，GalHub 会根据重定向规则将游戏文件夹重定向到指定文件夹\n"
                       "一个重定向规则由三部分组成：基文件夹、源文件夹相对路径、目标文件夹名称\n"
                       "例如，可以选择 %DOCUMENTS% 作为基文件夹，Leaf/WHITE ALBUM2 作为源文件夹相对路径，saves 作为目标文件夹名称\n"
                       "这样，GalHub 会将 %DOCUMENTS%/Leaf/WHITE ALBUM2 重定向到 Env/游戏名/saves 文件夹");
}

bool GameConfigRulesWidget::edit_rule (int row) {
    // 打开编辑对话框
    Rule rule = {
        rules_table_widget->item(row, 0)->text().toStdWString(),
        rules_table_widget->item(row, 1)->text().toStdWString(),
        rules_table_widget->item(row, 2)->text().toStdWString()
    };

    qDebug() << "edit rule";
    qDebug() << row << QString::fromStdWString(rule.src_base) << QString::fromStdWString(rule.src) << QString::fromStdWString(rule.dst_name);

    RuleDialog rule_dialog(_game_name, rule, this);
    rule_dialog.exec();

    // 更新配置
    if (rule_dialog.result() == QDialog::Accepted) {
        Rule new_rule = rule_dialog.rule();
        rules_table_widget->item(row, 0)->setText(QString::fromStdWString(new_rule.src_base));
        rules_table_widget->item(row, 1)->setText(QString::fromStdWString(new_rule.src));
        rules_table_widget->item(row, 2)->setText(QString::fromStdWString(new_rule.dst_name));
    } else {
        return false;
    }

    // 自适应列宽
    rules_table_widget->resizeColumnsToContents();

    // 重新绘制
    rules_table_widget->viewport()->update();

    return true;
}

void GameConfigRulesWidget::update_button_status () const {
    // 获取当前选中的 item
    auto *current_item = rules_table_widget->currentItem();
    if (current_item == nullptr) {
        // 没有选中的 item，禁用上移、下移、删除按钮
        up_button->setEnabled(false);
        down_button->setEnabled(false);
        del_button->setEnabled(false);
    } else {
        // 有选中的 item，根据当前 item 的位置，启用或禁用上移、下移、删除按钮
        int current_row = rules_table_widget->row(current_item);
        up_button->setEnabled(current_row != 0);
        down_button->setEnabled(current_row != rules_table_widget->rowCount() - 1);
        del_button->setEnabled(true);
    }
}


GameConfigRulesWidget::GameConfigRulesWidget (const QString &game_name, QWidget *parent)
    : QWidget(parent), _game_name(game_name) {
    init_ui();
}

GameConfigRulesWidget::GameConfigRulesWidget (const QString &game_name, const QList<Rule> &rules, QWidget *parent)
    : QWidget(parent), _game_name(game_name) {
    init_ui();
    load_rules(rules);
}

QList<Rule> GameConfigRulesWidget::rules () const {
    QList<Rule> rules;
    for (int i = 0; i < rules_table_widget->rowCount(); ++i) {
        auto *src_base_item = rules_table_widget->item(i, 0);
        auto *src_item = rules_table_widget->item(i, 1);
        auto *dst_item = rules_table_widget->item(i, 2);

        Rule rule;
        rule.src_base = src_base_item->text().toStdWString();
        rule.src = src_item->text().toStdWString();
        rule.dst_name = dst_item->text().toStdWString();

        rules.append(rule);
    }
    return rules;
}

void GameConfigRulesWidget::on_double_click (int row, int column) {
    edit_rule(row);
}

void GameConfigRulesWidget::on_table_item_selection_changed () const {
    update_button_status();
}

void GameConfigRulesWidget::on_table_item_changed () const {
    update_button_status();
}
