#include "MasterTableModel.h"

MasterTableModel::MasterTableModel(MasterRepo& repo, QObject* parent)
    : RepositoryTableModel(repo, {"ID", "Фамилия", "Имя", "Специализация", "Телефон"}, parent) {}

QVariant MasterTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    if (role == Qt::ToolTipRole) role = Qt::DisplayRole;     
    if (role != Qt::DisplayRole) return {};
    const Master& m = repo.at(static_cast<std::size_t>(index.row()));
    switch (index.column()) {
        case 0: return m.getId();
        case 1: return QString::fromStdString(m.getLastName());
        case 2: return QString::fromStdString(m.getFirstName());
        case 3: return QString::fromStdString(m.getSpecialization());
        case 4: return QString::fromStdString(m.getPhone());
    }
    return {};
}
