#pragma once
#include "RepositoryTableModel.h"
#include "Workshop.h"

class OrderTableModel : public RepositoryTableModel<Order, std::vector<Order>> {
public:
    explicit OrderTableModel(OrderRepo& repo, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role) const override;
};
