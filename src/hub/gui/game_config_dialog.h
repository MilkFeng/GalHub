#ifndef ADD_GAME_DIALOG_H
#define ADD_GAME_DIALOG_H

#include <QPushButton>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>


#include "../env_manager.h"

class GameConfigDialog : public QDialog {
    Q_OBJECT

    QString _input_game_name;

    QString _game_name;
    QString _thumbnail_path;
    QString _original_game_path;

    bool edit_mode;

    void init_ui ();

public:
    explicit GameConfigDialog (QWidget *parent = nullptr);
    explicit GameConfigDialog (const GameConfig &game_config, QWidget *parent = nullptr);

    [[nodiscard]] GameConfig game_config () const;

private slots:
    void on_ok_button_clicked ();
};


class GameConfigPathEdit : public QObject {
    Q_OBJECT

    QString &path;
    QString filter;

public:
    QLabel *label;
    QLineEdit *line_edit;
    QPushButton *button;

    explicit GameConfigPathEdit (QString &path, const QString &label_text = QString(), QString filter = QString(), QWidget *parent = nullptr);

private slots:
    void on_button_clicked () const;
    void on_line_edit_text_changed () const;
};

class GameConfigTextEdit : public QObject {
    Q_OBJECT

    QString &text;

public:
    QLabel *label;
    QLineEdit *line_edit;

    explicit GameConfigTextEdit (QString &text, const QString &label_text = QString(), QWidget *parent = nullptr);

private slots:
    void on_line_edit_text_changed () const;
};

#endif //ADD_GAME_DIALOG_H
