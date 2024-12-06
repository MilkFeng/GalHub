#ifndef GAME_INFO_RULES_WIDGET_H
#define GAME_INFO_RULES_WIDGET_H

#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

#include "../env_manager.h"

class GameConfigRulesWidget : public QWidget {
    Q_OBJECT

    QPushButton *up_button;
    QPushButton *down_button;
    QPushButton *add_button;
    QPushButton *del_button;

    QTableWidget *rules_table_widget;

    const QString &_game_name;

    void init_ui ();
    void clear_table () const;
    void load_rules (const QList<Rule> &rules) const;

    void swap_row (int row1, int row2) const;

    void add_rule_to (const Rule &rule, int row) const;
    void add_rule_to_end (const Rule &rule) const;

    void move_up () const;
    void move_down () const;
    void add_rule ();
    void del_rule () const;

    static void show_tool_tip ();

    bool edit_rule (int row);

    void update_button_status () const;

public:
    explicit GameConfigRulesWidget (const QString &game_name, QWidget *parent = nullptr);
    explicit GameConfigRulesWidget (const QString &game_name, const QList<Rule> &rules, QWidget *parent = nullptr);

    [[nodiscard]] QList<Rule> rules () const;

private slots:
    void on_double_click (int row, int column);
    void on_table_item_selection_changed () const;
    void on_table_item_changed () const;
};





#endif //GAME_INFO_RULES_WIDGET_H
