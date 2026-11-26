#include "StatusDelegate.h"

#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

StatusDelegate::StatusDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

void StatusDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QString text = opt.text;
    opt.text.clear();
    const QWidget* widget = option.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    QColor color = index.data(Qt::UserRole).value<QColor>();
    if (!color.isValid()) color = QColor("#7f8c8d");

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QRect pill = option.rect.adjusted(6, 7, -6, -7);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(pill, 5, 5);
    QFont font = option.font;
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::white);
    painter->drawText(pill, Qt::AlignCenter, text);
    painter->restore();
}
