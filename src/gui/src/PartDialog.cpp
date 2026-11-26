#include "PartDialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>

#include "Exceptions.h"
#include "Validation.h"

PartDialog::PartDialog(QWidget* parent, const Part* existing) : QDialog(parent) {
    setWindowTitle(existing ? "Редактирование детали" : "Новая деталь");
    nameEdit = new QLineEdit(this);
    categoryEdit = new QLineEdit(this);
    qtySpin = new QSpinBox(this);
    qtySpin->setRange(0, 100000);
    priceSpin = new QDoubleSpinBox(this);
    priceSpin->setRange(0, 100000);
    priceSpin->setDecimals(2);
    if (existing) {
        nameEdit->setText(QString::fromStdString(existing->getName()));
        categoryEdit->setText(QString::fromStdString(existing->getCategory()));
        qtySpin->setValue(existing->getQuantity());
        priceSpin->setValue(existing->getPrice());
    }

    auto* form = new QFormLayout(this);
    form->addRow("Наименование:", nameEdit);
    form->addRow("Категория:", categoryEdit);
    form->addRow("Количество:", qtySpin);
    form->addRow("Цена, BYN:", priceSpin);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &PartDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &PartDialog::reject);
}

void PartDialog::validate() const {
    if (!validation::isValidText(nameEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Наименование (не пусто, до 80 символов)");
}

void PartDialog::accept() {
    try {
        validate();
        QDialog::accept();
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Ошибка ввода", QString::fromUtf8(e.what()));
    }
}

Part PartDialog::getPart() const {
    return Part(0, nameEdit->text().trimmed().toStdString(),
                categoryEdit->text().trimmed().toStdString(), qtySpin->value(), priceSpin->value());
}
