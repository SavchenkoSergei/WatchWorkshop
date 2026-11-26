#pragma once
#include "RepositoryTableModel.h"
#include "Workshop.h"

// Таблица журнала действий (только для администратора).
class LogTableModel : public RepositoryTableModel<LogEntry, std::vector<LogEntry>> {
public:
    explicit LogTableModel(LogRepo& repo, QObject* parent = nullptr);
    QVariant data(const QModelIndex& index, int role) const override;
};
