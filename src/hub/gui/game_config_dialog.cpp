#include "game_config_dialog.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>

#include "game_config_rules_widget.h"
#include "../../common/util.h"


static const QDir &working_dir_qdir () {
    static QDir working_dir_ = QDir(working_dir());
    return working_dir_;
}

void GameConfigDialog::init_ui () {
    resize(600, 400);

    auto *layout = new QVBoxLayout(this);
    setLayout(layout);

    // 元数据部分

    auto *game_name_line_edit = new QLineEdit(_game_name);
    connect(game_name_line_edit, &QLineEdit::textChanged, [this](const QString &text) {
        _game_name = text;
    });

    auto *game_path_edit_group = new PathEditGroup(_original_game_path, working_dir_qdir(), "Executables (*.exe)", this);
    auto *thumbnail_edit_group = new PathEditGroup(_thumbnail_path, working_dir_qdir(), "Images (*.png *.jpg *.jpeg)", this);

    // connect(thumbnail_edit_group, &PathEditGroup::path_changed, this, &GameConfigDialog::on_thumbnail_path_changed);

    thumbnail = new QLabel(_thumbnail_path);
    thumbnail->setVisible(false);

    auto *meta_group = new QGroupBox("元数据");
    meta_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto *meta_frame_layout = new QGridLayout(this);
    meta_group->setLayout(meta_frame_layout);

    meta_frame_layout->addWidget(thumbnail, 0, 3, 3, 1);

    meta_frame_layout->addWidget(new QLabel("游戏名称"), 0, 0);
    meta_frame_layout->addWidget(game_name_line_edit, 0, 1);

    meta_frame_layout->addWidget(new QLabel("游戏路径"), 1, 0);
    meta_frame_layout->addWidget(game_path_edit_group->line_edit, 1, 1);
    meta_frame_layout->addWidget(game_path_edit_group->button, 1, 2);

    meta_frame_layout->addWidget(new QLabel("游戏缩略图"), 2, 0);
    meta_frame_layout->addWidget(thumbnail_edit_group->line_edit, 2, 1);
    meta_frame_layout->addWidget(thumbnail_edit_group->button, 2, 2);

    layout->addWidget(meta_group);

    // 规则部分

    QList<Rule> rules;
    if (edit_mode) {
        for (const auto &rule: _input_game_config.rules) {
            rules.append(rule);
        }
    }

    rules_widget = new GameConfigRulesWidget(_game_name, rules, this);

    auto *rule_group = new QGroupBox("重定向规则");
    rule_group->setMinimumHeight(200);
    auto *rule_group_layout = new QVBoxLayout(this);
    rule_group->setLayout(rule_group_layout);

    rule_group_layout->addWidget(rules_widget);

    layout->addWidget(rule_group);

    // 按钮部分

    auto *dialog_button_box = new QDialogButtonBox(this);
    dialog_button_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dialog_button_box, &QDialogButtonBox::accepted, this, &GameConfigDialog::on_ok_button_clicked);
    connect(dialog_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(dialog_button_box);
}

GameConfigDialog::GameConfigDialog (QWidget *parent) : QDialog(parent) {
    edit_mode = false;

    setWindowTitle("添加游戏");
    init_ui();
}

GameConfigDialog::GameConfigDialog (const GameConfig &game_config, QWidget *parent) : QDialog(parent) {
    _game_name = QString::fromStdWString(game_config.game_name);
    _thumbnail_path = QString::fromStdWString(game_config.thumbnail_path);
    _original_game_path = QString::fromStdWString(game_config.original_game_path);

    _input_game_config = game_config;

    edit_mode = true;
    _input_game_name = _game_name;

    setWindowTitle("编辑游戏：" + _game_name);
    init_ui();
}

GameConfig GameConfigDialog::game_config () const {
    Path thumbnail_path = _thumbnail_path.toStdWString();
    Path original_game_path = _original_game_path.toStdWString();
    String game_name = _game_name.toStdWString();

    QList<Rule> rules = rules_widget->rules();
    std::vector<Rule> rules_;
    for (const auto &rule: rules) {
        rules_.push_back(rule);
    }

    return {
        thumbnail_path,
        original_game_path,
        _game_name.toStdWString(),
        rules_
    };
}

void GameConfigDialog::on_ok_button_clicked () {
    auto game_config = GameConfigDialog::game_config();

    // 检查是否合法
    if (String message; !game_config.check(message)) {
        QMessageBox::warning(nullptr, "错误", QString::fromStdWString(message));
        return;
    }

    // 检查是否已存在，如果已存在，询问是否覆盖
    if (!edit_mode && EnvManager::instance().config().has_game(game_config.game_name)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            nullptr, "警告", "已存在同名游戏，是否覆盖？",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No) return;
    }

    // 更新配置
    EnvManager::instance().init_env_of_game(game_config);
    auto config = EnvManager::instance().config();

    if (edit_mode) {
        // 删除原有游戏配置
        config.del_game(_input_game_name.toStdWString());
    }
    config.upsert_game(game_config);

    qDebug() << "game_config.game_name: " << game_config.game_name;
    qDebug() << "game_config.rules.size(): " << game_config.rules.size();
    qDebug() << "game_config.thumbnail_path: " << game_config.thumbnail_path;
    qDebug() << "game_config.original_game_path: " << game_config.original_game_path;

    EnvManager::instance().upd_config(config);
    EnvManager::instance().write_config();

    // 关闭对话框
    accept();
}

void GameConfigDialog::on_thumbnail_path_changed (const QString &path) const {
    // 更新缩略图
    auto pixel = QPixmap(path).scaled(64, 64);
    qDebug() << path;
    if (pixel.isNull()) {
        thumbnail->setVisible(false);
    } else {
        thumbnail->setVisible(true);
        thumbnail->setPixmap(pixel);
    }
}
