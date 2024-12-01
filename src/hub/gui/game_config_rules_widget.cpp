#include "game_config_rules_widget.h"

void GameConfigRulesWidget::init_ui () {
    // 添加规则
    setSelectionMode(QAbstractItemView::SingleSelection);


}

GameConfigRulesWidget::GameConfigRulesWidget (QWidget *parent) : QListWidget(parent) {
    init_ui();
}

GameConfigRulesWidget::GameConfigRulesWidget (std::vector<Rule> rules, QWidget *parent)
    : QListWidget(parent), rules(std::move(rules)) {
    init_ui();
}
