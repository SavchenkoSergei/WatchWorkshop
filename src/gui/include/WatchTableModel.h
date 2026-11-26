#pragma once
#include "RepositoryTableModel.h"
#include "Workshop.h"

class WatchTableModel : public RepositoryTableModel<Watch, std::list<Watch>> {
public:
    explicit WatchTableModel(WatchRepo& repo, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role) const override;
};
