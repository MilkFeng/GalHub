#ifndef RULE_DIAG_H
#define RULE_DIAG_H

#include <QDialog>
#include <QDir>

#include "../../common/env.h"


class RuleDialog : public QDialog {
    Q_OBJECT

    QString _src_base;
    QString _src;
    QString _dst_name;

    const QString &game_name;

    static QStringList src_base_item ();

public:
    explicit RuleDialog (const QString &game_name, const Rule &rule, QWidget *parent = nullptr);

    [[nodiscard]] Rule rule () const;
};



#endif //RULE_DIAG_H
