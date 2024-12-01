#include "game_config_dialog.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "game_config_rules_widget.h"


void GameConfigDialog::init_ui () {
    resize(600, 400);

    auto *layout = new QVBoxLayout(this);
    setLayout(layout);

    // 元数据部分

    auto *game_name_text_edit = new GameConfigTextEdit(_game_name, "游戏名称", this);
    auto *game_path_edit = new GameConfigPathEdit(_original_game_path, "游戏路径", "Executables (*.exe)", this);
    auto *thumbnail_edit = new GameConfigPathEdit(_thumbnail_path, "游戏缩略图", "Images (*.png *.jpg *.jpeg)", this);

    auto *meta_group = new QGroupBox("元数据");
    meta_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto *meta_frame_layout = new QGridLayout(this);
    meta_group->setLayout(meta_frame_layout);

    meta_frame_layout->addWidget(game_name_text_edit->label, 0, 0);
    meta_frame_layout->addWidget(game_name_text_edit->line_edit, 0, 1);

    meta_frame_layout->addWidget(game_path_edit->label, 1, 0);
    meta_frame_layout->addWidget(game_path_edit->line_edit, 1, 1);
    meta_frame_layout->addWidget(game_path_edit->button, 1, 2);

    meta_frame_layout->addWidget(thumbnail_edit->label, 2, 0);
    meta_frame_layout->addWidget(thumbnail_edit->line_edit, 2, 1);
    meta_frame_layout->addWidget(thumbnail_edit->button, 2, 2);

    layout->addWidget(meta_group);

    // 规则部分

    auto *rules_widget = new GameConfigRulesWidget(this);

    auto *rule_group = new QGroupBox("规则");
    rule_group->setMinimumHeight(200);
    auto *rule_frame_layout = new QVBoxLayout(this);
    rule_group->setLayout(rule_frame_layout);

    rule_frame_layout->addWidget(rules_widget);

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

    edit_mode = true;
    _input_game_name = _game_name;

    setWindowTitle("编辑游戏：" + _game_name);
    init_ui();
}

GameConfig GameConfigDialog::game_config () const {
    return {
        _thumbnail_path.toStdWString(),
        _original_game_path.toStdWString(),
        _game_name.toStdWString()
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

    EnvManager::instance().upd_config(config);
    EnvManager::instance().write_config();

    // 关闭对话框
    accept();
}

GameConfigPathEdit::GameConfigPathEdit (QString &path, const QString &label_text, QString filter, QWidget *parent)
    : QObject(parent), path(path), filter(std::move(filter)) {
    label = new QLabel(label_text);
    line_edit = new QLineEdit(path);
    button = new QPushButton("...");

    connect(button, &QPushButton::clicked, this, &GameConfigPathEdit::on_button_clicked);
    connect(line_edit, &QLineEdit::textChanged, this, &GameConfigPathEdit::on_line_edit_text_changed);
}

void GameConfigPathEdit::on_button_clicked () const {
    QString path = QFileDialog::getOpenFileName(nullptr, "选择文件", nullptr, filter);
    if (!path.isEmpty()) line_edit->setText(path);
}

void GameConfigPathEdit::on_line_edit_text_changed () const {
    path = line_edit->text();
}

GameConfigTextEdit::GameConfigTextEdit (QString &text, const QString &label_text, QWidget *parent)
    : QObject(parent), text(text) {
    label = new QLabel(label_text);
    line_edit = new QLineEdit(text);

    connect(line_edit, &QLineEdit::textChanged, this, &GameConfigTextEdit::on_line_edit_text_changed);
}

void GameConfigTextEdit::on_line_edit_text_changed () const {
    text = line_edit->text();
}
