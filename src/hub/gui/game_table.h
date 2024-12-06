#ifndef GAME_TABLE_H
#define GAME_TABLE_H

#include <QTableWidget>
#include <QDir>
#include <QList>

#include "../../common/env.h"

struct GameItem {
    QPixmap thumbnail;
    QString name;
    QDir path;
    QList<Rule> rules;
};

class GameTableWidget : public QTableWidget {
    Q_OBJECT

    void clear_table ();
    void add_game_item (const GameItem &game_item);

    void show_context_menu (const QPoint &pos, int row);

    void sort_by_column (int column);
public:
    explicit GameTableWidget(QWidget *parent = nullptr);

private slots:
    void on_config_upd ();

    void on_right_click (const QPoint &pos);
    void on_double_click (int row, int column) const;

    void on_edit_action_triggered (int row) const;
    void on_delete_action_triggered (int row) const;
};


#endif //GAME_TABLE_H
