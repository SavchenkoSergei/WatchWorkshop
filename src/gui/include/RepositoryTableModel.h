#pragma once
// Шаблонная база всех табличных моделей: хранит ссылку на репозиторий и названия
// столбцов. Конкретные модели реализуют только data().
// Q_OBJECT в шаблонных классах не поддерживается, но здесь он и не нужен:
// собственных сигналов и слотов у моделей нет.
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
    // Вызывается после изменения данных: таблица перечитывает репозиторий.
    void refresh() {
        beginResetModel();
        endResetModel();
    }

protected:
    Repository<T, Container>& repo;
    QStringList headers;
};
