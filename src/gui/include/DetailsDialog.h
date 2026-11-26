#pragma once
#include <QDialog>

// Окно просмотра полной информации о записи (заказ, часы): текст неисправности
// и другие длинные значения показываются целиком, а не обрезаются в таблице.
class DetailsDialog : public QDialog {
public:
    DetailsDialog(const QString& title, const QString& html, QWidget* parent = nullptr);
};
