#include "OrderTableModel.h"

#include <QBrush>
#include <QColor>

#include "Dates.h"

namespace {

QColor statusColor(OrderStatus s) {
    switch (s) {
        case OrderStatus::Accepted:    return QColor("#3498db");   // синий
        case OrderStatus::Diagnostics: return QColor("#f39c12");   // оранжевый
        case OrderStatus::InProgress:  return QColor("#27ae60");   // зелёный
        case OrderStatus::Ready:       return QColor("#16a085");   // бирюзовый
        case OrderStatus::Issued:      return QColor("#7f8c8d");   // серый
    }
    return QColor("#7f8c8d");
}

QString personName(const Client* c) {
    return c ? QString::fromStdString(c->getLastName() + " " + c->getFirstName()) : QString("—");
}

} 

OrderTableModel::OrderTableModel(OrderRepo& repo, QObject* parent)
    : RepositoryTableModel(repo, {"ID", "Клиент", "Часы", "Мастер", "Услуги", "Статус", "Принят",
                                  "Плановая дата", "Осталось", "Выдан", "Стоимость, BYN"}, parent) {}

QVariant OrderTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const Order& o = itemAt(static_cast<std::size_t>(index.row()));
    bool finished = o.getStatus() >= OrderStatus::Ready;         
    int left = o.daysLeft(dates::today());                        

    if (role == Qt::ToolTipRole) role = Qt::DisplayRole;      
    if (role == Qt::UserRole && index.column() == 5) return statusColor(o.getStatus());
    if (role == Qt::UserRole + 1 && index.column() == 8)     
        return finished ? 100000 : left;
    if (role == Qt::ForegroundRole && index.column() == 8 && !finished) {
        if (left < 0) return QBrush(QColor("#c0392b"));           // просрочен
        if (left <= 1) return QBrush(QColor("#d68910"));          // срок поджимает
        return {};
    }
    if (role != Qt::DisplayRole) return {};

    switch (index.column()) {
        case 0: return o.getId();
        case 1: return personName(o.getClient());
        case 2: return o.getWatch()
                    ? QString::fromStdString(o.getWatch()->getBrand() + " " + o.getWatch()->getModel())
                    : QString("—");
        case 3: return o.getMaster()
                    ? QString::fromStdString(o.getMaster()->getLastName() + " " + o.getMaster()->getFirstName())
                    : QString("не назначен");
        case 4: {
            QStringList names;
            for (const auto& s : o.getServices()) names << QString::fromStdString(s->getName());
            return names.join(", ");
        }
        case 5: return QString::fromStdString(statusToString(o.getStatus()));
        case 6: return QString::fromStdString(o.getAcceptDate());
        case 7: return QString::fromStdString(o.plannedDate());
        case 8:
            if (finished) return QString("—");
            if (left < 0) return QString("просрочен на %1 дн.").arg(-left);
            if (left == 0) return QString("сегодня");
            return QString("%1 дн.").arg(left);
        case 9: return o.getIssueDate().empty() ? QString("—") : QString::fromStdString(o.getIssueDate());
        case 10: return QString::number(o.totalCost(), 'f', 2);
    }
    return {};
}
