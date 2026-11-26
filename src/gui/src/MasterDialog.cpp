#include "MasterDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>

#include "Exceptions.h"
#include "Validation.h"

MasterDialog::MasterDialog(QWidget* parent, const Master* existing) : QDialog(parent) {
    setWindowTitle(existing ? "Редактирование мастера" : "Новый мастер");
    lastNameEdit = new QLineEdit(this);
    firstNameEdit = new QLineEdit(this);
    specializationEdit = new QLineEdit(this);
    phoneEdit = new QLineEdit(this);
    if (existing) {
        lastNameEdit->setText(QString::fromStdString(existing->getLastName()));
        firstNameEdit->setText(QString::fromStdString(existing->getFirstName()));
        specializationEdit->setText(QString::fromStdString(existing->getSpecialization()));
        phoneEdit->setText(QString::fromStdString(existing->getPhone()));
    }

    connect(phoneEdit, &QLineEdit::editingFinished, this, [this] {
        phoneEdit->setText(QString::fromStdString(
            validation::formatPhone(phoneEdit->text().trimmed().toStdString())));
    });

    auto* form = new QFormLayout(this);
    form->addRow("Фамилия:", lastNameEdit);
    form->addRow("Имя:", firstNameEdit);
    form->addRow("Специализация:", specializationEdit);
    form->addRow("Телефон:", phoneEdit);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &MasterDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &MasterDialog::reject);
}

void MasterDialog::validate() const {
    using namespace validation;
    if (!isValidName(lastNameEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Фамилия (только буквы, пробел и дефис)");
    if (!isValidName(firstNameEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Имя (только буквы, пробел и дефис)");
    std::string phone = phoneEdit->text().trimmed().toStdString();
    if (!phone.empty() && !isValidPhone(phone))
        throw InvalidInputException("Телефон (7–15 цифр или оставьте пустым)");
}

void MasterDialog::accept() {
    try {
        validate();
        QDialog::accept();
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Ошибка ввода", QString::fromUtf8(e.what()));
    }
}

Master MasterDialog::getMaster() const {
    return Master(0, lastNameEdit->text().trimmed().toStdString(),
                  firstNameEdit->text().trimmed().toStdString(),
                  specializationEdit->text().trimmed().toStdString(),
                  phoneEdit->text().trimmed().toStdString());
}
