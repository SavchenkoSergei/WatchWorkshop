#pragma once
#include "RepositoryTableModel.h"
#include "Workshop.h"

class MasterTableModel : public RepositoryTableModel<Master, std::list<Master>> {
public:
    explicit MasterTableModel(MasterRepo& repo, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role) const override;
};
