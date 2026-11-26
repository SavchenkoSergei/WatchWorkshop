#pragma once
#include <QDialog>

class DetailsDialog : public QDialog {
public:
    DetailsDialog(const QString& title, const QString& html, QWidget* parent = nullptr);
};
