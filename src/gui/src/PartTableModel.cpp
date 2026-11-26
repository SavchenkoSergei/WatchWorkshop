#include "PartTableModel.h"

PartTableModel::PartTableModel(PartRepo& repo, QObject* parent)
    : RepositoryTableModel(repo, {"ID", "Наименование", "Категория", "Остаток", "Цена, BYN"}, parent) {}

QVariant PartTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    if (role == Qt::ToolTipRole) role = Qt::DisplayRole;    
    if (role != Qt::DisplayRole) return {};
    const Part& p = repo.at(static_cast<std::size_t>(index.row()));
    switch (index.column()) {
        case 0: return p.getId();
        case 1: return QString::fromStdString(p.getName());
        case 2: return QString::fromStdString(p.getCategory());
        case 3: return p.getQuantity();
        case 4: return QString::number(p.getPrice(), 'f', 2);
    }
    return {};
}
