#ifndef GAME_INFO_RULES_WIDGET_H
#define GAME_INFO_RULES_WIDGET_H

#include <QListWidget>

#include "../env_manager.h"

class GameConfigRulesWidget : public QListWidget {
    Q_OBJECT

    std::vector<Rule> rules;

    void init_ui ();

public:
    explicit GameConfigRulesWidget (QWidget *parent = nullptr);
    explicit GameConfigRulesWidget (std::vector<Rule> rules, QWidget *parent = nullptr);
};



#endif //GAME_INFO_RULES_WIDGET_H
