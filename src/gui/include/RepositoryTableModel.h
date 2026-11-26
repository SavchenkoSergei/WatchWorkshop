#pragma once
#include <cstddef>

#include <QAbstractTableModel>
#include <QStringList>

#include "Repository.h"

template <typename T, typename Container>
class RepositoryTableModel : public QAbstractTableModel {
public:
    RepositoryTableModel(Repository<T, Container>& repo, QStringList headers,
        QObject* parent = nullptr)
        : QAbstractTableModel(parent), repo_(repo), headers_(std::move(headers)) {
    }

    int rowCount(const QModelIndex & = {}) const override {
        return static_cast<int>(repo_.size());
    }
    int columnCount(const QModelIndex & = {}) const override {
        return static_cast<int>(headers_.size());
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return headers_.value(section);
        return {};
    }
    void refresh() {
        beginResetModel();
        endResetModel();
    }

protected:
    const T& itemAt(std::size_t row) const { return repo_.at(row); }
    T& itemAt(std::size_t row) { return repo_.at(row); }
    const Repository<T, Container>& repository() const { return repo_; }
    const QStringList& headersList() const { return headers_; }

private:
    Repository<T, Container>& repo_;
    QStringList headers_;
};