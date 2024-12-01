#ifndef GAME_TABLE_H
#define GAME_TABLE_H

#include <QTableWidget>

struct GameItem {
    QPixmap thumbnail;
    QString name;
    QString path;
};


class GameTable : public QTableWidget {
    Q_OBJECT

public:
    explicit GameTable(QWidget *parent = nullptr);

private slots:
    void on_config_upd ();
};



#endif //GAME_TABLE_H
