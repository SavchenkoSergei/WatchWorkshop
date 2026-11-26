#include "SortProxy.h"

#include <numeric>
#include <vector>

#include "Algorithms.h"

SortProxy::SortProxy(QObject* parent) : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(-1);              // поиск по всем столбцам
    // сортировка включена всегда и выполняется через lessThan (по рангам строк);
    // при смене порядка достаточно вызвать invalidate()
    QSortFilterProxyModel::sort(0, Qt::AscendingOrder);
}

void SortProxy::setSource(QAbstractItemModel* model) {
    setSourceModel(model);
    connect(model, &QAbstractItemModel::modelReset, this, [this] { resort(); });
}

void SortProxy::toggleColumn(int column) {
    if (column == sortColumn) sortOrder = (sortOrder == Qt::DescendingOrder) ? Qt::AscendingOrder
                                                                             : Qt::DescendingOrder;
    else {
        sortColumn = column;
        sortOrder = Qt::DescendingOrder;
    }
    resort();
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}

// Значения сравниваются как числа, если оба значения числовые (ID, количество, цена, срок),
// иначе как строки с учётом локали.
bool SortProxy::lessValue(int rowA, int rowB) const {
    QAbstractItemModel* src = sourceModel();
    QVariant a = src->index(rowA, sortColumn).data(Qt::UserRole + 1);    // числовой ключ, если модель его даёт
    QVariant b = src->index(rowB, sortColumn).data(Qt::UserRole + 1);
    if (!a.isValid() || !b.isValid()) {
        a = src->index(rowA, sortColumn).data(Qt::DisplayRole);
        b = src->index(rowB, sortColumn).data(Qt::DisplayRole);
    }
    bool okA = false, okB = false;
    double da = a.toDouble(&okA), db = b.toDouble(&okB);
    if (okA && okB) return da < db;
    return QString::localeAwareCompare(a.toString(), b.toString()) < 0;
}

void SortProxy::resort() {
    QAbstractItemModel* src = sourceModel();
    if (!src || sortColumn < 0 || sortColumn >= src->columnCount()) {
        rank.clear();
        invalidate();                                         // исходный порядок
        return;
    }
    std::vector<int> rows(static_cast<std::size_t>(src->rowCount()));
    std::iota(rows.begin(), rows.end(), 0);
    bool descending = (sortOrder == Qt::DescendingOrder);
    algo::mergeSort(rows, [this, descending](int a, int b) {  // собственный алгоритм сортировки
        return descending ? lessValue(b, a) : lessValue(a, b);
    });
    rank = QVector<int>(src->rowCount(), 0);
    for (std::size_t i = 0; i < rows.size(); ++i) rank[rows[i]] = static_cast<int>(i);
    invalidate();                                             // применяет новый порядок через lessThan
}

bool SortProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    if (rank.isEmpty()) return left.row() < right.row();
    return rank.value(left.row()) < rank.value(right.row());
}

// Стрелка в заголовке показывает столбец и направление сортировки.
QVariant SortProxy::headerData(int section, Qt::Orientation orientation, int role) const {
    QVariant v = QSortFilterProxyModel::headerData(section, orientation, role);
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == sortColumn)
        return v.toString() + (sortOrder == Qt::DescendingOrder ? "  ▼" : "  ▲");
    return v;
}
