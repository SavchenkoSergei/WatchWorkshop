#pragma once
#include "RepositoryTableModel.h"
#include "Workshop.h"

class ClientTableModel : public RepositoryTableModel<Client, std::list<Client>> {
public:
    explicit ClientTableModel(ClientRepo& repo, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role) const override;
};
