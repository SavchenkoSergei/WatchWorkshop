#include "WatchTableModel.h"

WatchTableModel::WatchTableModel(WatchRepo& repo, QObject* parent)
    : RepositoryTableModel(repo, {"ID", "Владелец", "Марка", "Модель", "Серийный номер",
                                  "Механизм", "Неисправность"}, parent) {}

QVariant WatchTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    if (role == Qt::ToolTipRole) role = Qt::DisplayRole;      // полный текст ячейки по наведению мыши
    if (role != Qt::DisplayRole) return {};
    const Watch& w = repo.at(static_cast<std::size_t>(index.row()));
    switch (index.column()) {
        case 0: return w.getId();
        case 1: return w.getClient()
                    ? QString::fromStdString(w.getClient()->getLastName() + " " + w.getClient()->getFirstName())
                    : QString("—");
        case 2: return QString::fromStdString(w.getBrand());
        case 3: return QString::fromStdString(w.getModel());
        case 4: return QString::fromStdString(w.getSerialNumber());
        case 5: return QString::fromStdString(w.getMechanismType());
        case 6: return QString::fromStdString(w.getDefectDescription());
    }
    return {};
}
