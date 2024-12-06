#include "rule_diag.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

#include "path_edit_group.h"
#include "../env_manager.h"

QStringList RuleDialog::src_base_item () {
    static bool initialized = false;
    static QStringList items;

    if (!initialized) {
        for (const auto &item: Env::folder_vars()) {
            items.append(QString::fromStdWString(item));
        }
        initialized = true;
    }

    return items;
}

RuleDialog::RuleDialog (const QString &game_name, const Rule &rule, QWidget *parent)
    : QDialog(parent), game_name(game_name) {
    setWindowTitle("编辑或添加重定向规则");

    auto *layout = new QVBoxLayout(this);
    setLayout(layout);

    // 编辑部分

    auto *widget = new QWidget(this);
    auto *widget_layout = new QGridLayout(widget);
    widget_layout->setContentsMargins(0, 0, 0, 0);


    _src_base = QString::fromStdWString(rule.src_base);
    _src = QString::fromStdWString(rule.src);
    _dst_name = QString::fromStdWString(rule.dst_name);

    qDebug() << _src_base << _src << _dst_name;

    auto *src_base_combo_box = new QComboBox();
    src_base_combo_box->addItems(src_base_item());
    src_base_combo_box->setCurrentText(_src_base);
    connect(src_base_combo_box, &QComboBox::currentIndexChanged, [this](const int index) {
        _src_base = src_base_item()[index];
    });

    auto *src_path_line_edit = new QLineEdit(_src);
    connect(src_path_line_edit, &QLineEdit::textChanged, [this](const QString &text) {
        _src = text;
    });

    auto *dst_name_line_edit = new QLineEdit(_dst_name);
    connect(dst_name_line_edit, &QLineEdit::textChanged, [this](const QString &text) {
        _dst_name = text;
    });

    widget_layout->addWidget(new QLabel("基文件夹"), 0, 0);
    widget_layout->addWidget(src_base_combo_box, 0, 1);

    widget_layout->addWidget(new QLabel("源文件夹相对路径"), 1, 0);
    widget_layout->addWidget(src_path_line_edit, 1, 1);

    widget_layout->addWidget(new QLabel("目标文件夹名称"), 2, 0);
    widget_layout->addWidget(dst_name_line_edit, 2, 1);


    layout->addWidget(widget);

    // 按钮部分
    auto *dialog_button_box = new QDialogButtonBox(this);
    dialog_button_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(dialog_button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(dialog_button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(dialog_button_box);
}

Rule RuleDialog::rule () const {
    return {
        _src_base.toStdWString(),
        _src.toStdWString(),
        _dst_name.toStdWString()
    };
}
