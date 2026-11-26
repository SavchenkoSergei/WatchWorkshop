#pragma once

#include <QSortFilterProxyModel>
#include <QVector>

class SortProxy : public QSortFilterProxyModel {
public:
    explicit SortProxy(QObject* parent = nullptr);

    void setSource(QAbstractItemModel* model);

    void toggleColumn(int column);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    void resort();                 
    bool lessValue(int rowA, int rowB) const;

    int sortColumn = -1;
    Qt::SortOrder sortOrder = Qt::DescendingOrder;
    QVector<int> rank;                  
};
