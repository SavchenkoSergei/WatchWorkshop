#pragma once
#include <QDialog>

#include "Part.h"

class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;

class PartDialog : public QDialog {
public:
    explicit PartDialog(QWidget* parent = nullptr, const Part* existing = nullptr);
    Part getPart() const;
    void accept() override;

private:
    void validate() const;

    QLineEdit* nameEdit;
    QLineEdit* categoryEdit;
    QSpinBox* qtySpin;
    QDoubleSpinBox* priceSpin;
};
