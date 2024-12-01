#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow (QWidget *parent = nullptr);

private slots:
    void on_add_game_action_triggered ();
};


#endif //MAIN_WINDOW_H
