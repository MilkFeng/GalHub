#include "auto_deselect_event_filter.h"

#include <QMouseEvent>


void AutoDeselectEventFilter::deselect (QMouseEvent *e) const {
    // 获取当前鼠标坐标
    QPoint point = e->pos();

    // 根据鼠标坐标，获取此时鼠标按下时所在的行、列
    QModelIndex index = table->indexAt(point);

    // 判断该单元格是否是空单元格
    if (table->item(index.row(), index.column()) == nullptr) {
        //取消选中
        table->setCurrentItem(nullptr);
    }
}

bool AutoDeselectEventFilter::eventFilter (QObject *watched, QEvent *event) {
    // 类型转换
    auto *e = dynamic_cast<QMouseEvent *>(event);
    // 判空
    if (e == nullptr) {
        return QObject::eventFilter(watched, event);
    }
    // 判断事件类型为鼠标点击
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
        // 点击空白处取消选中
        deselect(e);
    }

    return QObject::eventFilter(watched, event);
}

AutoDeselectEventFilter::AutoDeselectEventFilter (QTableWidget *table) : QObject(table), table(table) {
}
