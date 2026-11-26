#pragma once
// Прокси-модель для таблиц: поиск по любому столбцу и сортировка по щелчку на заголовок
// собственным алгоритмом сортировки слиянием (algo::mergeSort).
// Первый щелчок по столбцу сортирует по убыванию, следующий меняет направление.
#include <QSortFilterProxyModel>
#include <QVector>

class SortProxy : public QSortFilterProxyModel {
public:
    explicit SortProxy(QObject* parent = nullptr);

    void setSource(QAbstractItemModel* model);
    // Щелчок по заголовку столбца: выбирает направление и сортирует.
    void toggleColumn(int column);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    void resort();                       // пересчитывает порядок строк (в том числе после обновления данных)
    bool lessValue(int rowA, int rowB) const;

    int sortColumn = -1;
    Qt::SortOrder sortOrder = Qt::DescendingOrder;
    QVector<int> rank;                   // место каждой строки источника в отсортированном порядке
};
