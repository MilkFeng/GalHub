#ifndef AUTO_DESELECT_EVENT_FILTER_H
#define AUTO_DESELECT_EVENT_FILTER_H

#include <QTableWidget>


class AutoDeselectEventFilter : public QObject {
    Q_OBJECT

    QTableWidget *table;

    void deselect (QMouseEvent *e) const;

protected:
    bool eventFilter (QObject *watched, QEvent *event) override;

public:
    explicit AutoDeselectEventFilter (QTableWidget *table);
};

#endif //AUTO_DESELECT_EVENT_FILTER_H
