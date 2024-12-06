#include "path_edit_group.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <utility>

#include "../../common/util.h"


void PathEditGroup::init () {
    line_edit = new QLineEdit(path);
    button = new QPushButton("...");
    button->setFixedWidth(30);

    connect(button, &QPushButton::clicked, this, &PathEditGroup::on_button_clicked);
    connect(line_edit, &QLineEdit::textChanged, this, &PathEditGroup::on_line_edit_text_changed);
}

PathEditGroup::PathEditGroup (QString &path, const QDir &base, QString filter, QWidget *parent)
    : QObject(parent), path(path), filter(std::move(filter)), base(base), dir(false) {
    init();
}

PathEditGroup::PathEditGroup (QString &path, const QDir &base, QWidget *parent)
    : QObject(parent), path(path), base(base), dir(true) {
    init();
}

void PathEditGroup::update_path () const {
    line_edit->setText(path);
}

void PathEditGroup::on_button_clicked () const {
    qDebug() << "on_button_clicked invoked";
    qDebug() << base;
    qDebug() << line_edit->text();
    QString file_dialog_dir;

    if (base.isEmpty()) {
        file_dialog_dir = "";
    } else {
        try {
            if (line_edit->text().isEmpty()) {
                file_dialog_dir = base.absolutePath();
            } else {
                file_dialog_dir = base.absoluteFilePath(line_edit->text());
            }
        } catch (...) {
            qDebug() << "Error: " << __FILE__ << ":" << __LINE__;
            file_dialog_dir = base.absolutePath();
        }
    }

    QString path;
    if (dir) {
        path = QFileDialog::getExistingDirectory(nullptr, "选择文件夹", file_dialog_dir);
    } else {
        path = QFileDialog::getOpenFileName(nullptr, "选择文件", file_dialog_dir, filter);
    }

    if (!path.isEmpty()) {
        Path path_ = path.toStdWString();
        Path relative_path = path_.lexically_relative(base.path().toStdWString());

        line_edit->setText(QString::fromStdWString(relative_path));
    }
}

void PathEditGroup::on_line_edit_text_changed () const {
    path = line_edit->text();
    emit path_changed(base.absoluteFilePath(path));
}
