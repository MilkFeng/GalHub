#ifndef ADD_GAME_DIALOG_H
#define ADD_GAME_DIALOG_H

#include <QDialog>
#include <QLabel>

#include "path_edit_group.h"
#include "game_config_rules_widget.h"
#include "../env_manager.h"

class GameConfigDialog : public QDialog {
    Q_OBJECT

    QString _input_game_name;

    QString _game_name;
    QString _thumbnail_path;
    QString _original_game_path;

    GameConfigRulesWidget *rules_widget;

    GameConfig _input_game_config;

    QLabel *thumbnail;


    bool edit_mode;

    void init_ui ();

public:
    explicit GameConfigDialog (QWidget *parent = nullptr);
    explicit GameConfigDialog (const GameConfig &game_config, QWidget *parent = nullptr);

    [[nodiscard]] GameConfig game_config () const;

private slots:
    void on_ok_button_clicked ();
    void on_thumbnail_path_changed (const QString &path) const;
};

#endif //ADD_GAME_DIALOG_H
