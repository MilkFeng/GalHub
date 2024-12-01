#ifndef GAME_TABLE_H
#define GAME_TABLE_H

#include <QTableWidget>
#include <QDir>

struct GameItem {
    QPixmap thumbnail;
    QString name;
    QDir path;
};

class GameTable : public QTableWidget {
    Q_OBJECT

    void clear_table ();
    void add_game_item (const GameItem &game_item);

    void show_context_menu (const QPoint &pos, int row);

public:
    explicit GameTable(QWidget *parent = nullptr);

private slots:
    void on_config_upd ();

    void on_right_click (const QPoint &pos);
    void on_double_click (int row, int column) const;

    void on_edit_action_triggered (int row) const;
    void on_delete_action_triggered (int row) const;
};

class GameTableEventFilter : public QObject {
    Q_OBJECT

    GameTable *table;

    void deselect (QMouseEvent *e) const;

protected:
    bool eventFilter (QObject *watched, QEvent *event) override;

public:
    explicit GameTableEventFilter (GameTable *table);
};


#endif //GAME_TABLE_H
