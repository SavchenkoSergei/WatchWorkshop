#pragma once
#include "RepositoryTableModel.h"
#include "Workshop.h"

class PartTableModel : public RepositoryTableModel<Part, std::list<Part>> {
public:
    explicit PartTableModel(PartRepo& repo, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role) const override;
};
