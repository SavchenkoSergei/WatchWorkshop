#include "LogTableModel.h"

LogTableModel::LogTableModel(LogRepo& repo, QObject* parent)
    : RepositoryTableModel(repo, {"№", "Время", "Пользователь", "Роль", "Действие", "Подробности"}, parent) {}

QVariant LogTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    if (role == Qt::ToolTipRole) role = Qt::DisplayRole;      // полный текст ячейки по наведению мыши
    if (role != Qt::DisplayRole) return {};
    const LogEntry& e = itemAt(static_cast<std::size_t>(index.row()));
    switch (index.column()) {
        case 0: return e.getId();
        case 1: return QString::fromStdString(e.getTime());
        case 2: return QString::fromStdString(e.getLogin());
        case 3: return QString::fromStdString(roleToString(e.getRole()));
        case 4: return QString::fromStdString(e.getAction());
        case 5: return QString::fromStdString(e.getDetails());
    }
    return {};
}
