#include "ClientTableModel.h"

ClientTableModel::ClientTableModel(ClientRepo& repo, QObject* parent)
    : RepositoryTableModel(repo, {"ID", "Фамилия", "Имя", "Телефон", "Email"}, parent) {}

QVariant ClientTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    if (role == Qt::ToolTipRole) role = Qt::DisplayRole;      // полный текст ячейки по наведению мыши
    if (role != Qt::DisplayRole) return {};
    const Client& c = repo.at(static_cast<std::size_t>(index.row()));
    switch (index.column()) {
        case 0: return c.getId();
        case 1: return QString::fromStdString(c.getLastName());
        case 2: return QString::fromStdString(c.getFirstName());
        case 3: return QString::fromStdString(c.getPhone());
        case 4: return QString::fromStdString(c.getEmail());
    }
    return {};
}
