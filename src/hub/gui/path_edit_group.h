#ifndef PATH_EDIT_GROUP_H
#define PATH_EDIT_GROUP_H

#include <QString>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>


#include "../../common/env.h"
#include "../../common/util.h"


class PathEditGroup : public QObject {
    Q_OBJECT

    QString &path;
    QString filter;

    const QDir &base;
    bool dir;

    void init();

public:
    QLineEdit *line_edit;
    QPushButton *button;

    explicit PathEditGroup (QString &path, const QDir &base = working_dir(), QString filter = QString(), QWidget *parent = nullptr);
    explicit PathEditGroup (QString &path, const QDir &base = working_dir(), QWidget *parent = nullptr);

    void update_path () const;

private slots:
    void on_button_clicked () const;
    void on_line_edit_text_changed () const;

signals:
    void path_changed (const QString &path) const;
};

#endif //PATH_EDIT_GROUP_H
