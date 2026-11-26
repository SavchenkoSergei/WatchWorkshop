#pragma once
#include <QStyledItemDelegate>

// Рисует значение ячейки в виде цветной «плашки» (как статус заказа в CRM-системах).
// Цвет плашки модель отдаёт по роли Qt::UserRole.
class StatusDelegate : public QStyledItemDelegate {
public:
    explicit StatusDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};
