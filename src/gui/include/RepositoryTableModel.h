#pragma once
#include <QAbstractTableModel>
#include <QStringList>

#include "Repository.h"

template <typename T, typename Container>
class RepositoryTableModel : public QAbstractTableModel {
public:
    RepositoryTableModel(Repository<T, Container>& repo, QStringList headers,
                         QObject* parent = nullptr)
        : QAbstractTableModel(parent), repo(repo), headers(std::move(headers)) {}

    int rowCount(const QModelIndex& = {}) const override {
        return static_cast<int>(repo.size());
    }
    int columnCount(const QModelIndex& = {}) const override {
        return static_cast<int>(headers.size());
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return headers.value(section);
        return {};
    }
    void refresh() {
        beginResetModel();
        endResetModel();
    }

protected:
    Repository<T, Container>& repo;
    QStringList headers;
};
