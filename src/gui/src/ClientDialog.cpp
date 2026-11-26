#include "ClientDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>

#include "Exceptions.h"
#include "Validation.h"

ClientDialog::ClientDialog(QWidget* parent, const Client* existing) : QDialog(parent) {
    setWindowTitle(existing ? "Редактирование клиента" : "Новый клиент");
    lastNameEdit = new QLineEdit(this);
    firstNameEdit = new QLineEdit(this);
    phoneEdit = new QLineEdit(this);
    emailEdit = new QLineEdit(this);
    if (existing) {
        lastNameEdit->setText(QString::fromStdString(existing->getLastName()));
        firstNameEdit->setText(QString::fromStdString(existing->getFirstName()));
        phoneEdit->setText(QString::fromStdString(existing->getPhone()));
        emailEdit->setText(QString::fromStdString(existing->getEmail()));
    }

    // телефон приводится к виду «+375 29 824 9433», как только пользователь закончил ввод
    connect(phoneEdit, &QLineEdit::editingFinished, this, [this] {
        phoneEdit->setText(QString::fromStdString(
            validation::formatPhone(phoneEdit->text().trimmed().toStdString())));
    });

    auto* form = new QFormLayout(this);
    form->addRow("Фамилия:", lastNameEdit);
    form->addRow("Имя:", firstNameEdit);
    form->addRow("Телефон:", phoneEdit);
    form->addRow("Email:", emailEdit);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &ClientDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ClientDialog::reject);
}

void ClientDialog::validate() const {
    using namespace validation;
    if (!isValidName(lastNameEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Фамилия (только буквы, пробел и дефис)");
    if (!isValidName(firstNameEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Имя (только буквы, пробел и дефис)");
    if (!isValidPhone(phoneEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Телефон (7–15 цифр, например +375 29 111-22-33)");
    if (!isValidEmail(emailEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Email (вид name@mail.by или оставьте пустым)");
}

void ClientDialog::accept() {
    try {
        validate();
        QDialog::accept();
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Ошибка ввода", QString::fromUtf8(e.what()));
    }
}

Client ClientDialog::getClient() const {
    return Client(0, lastNameEdit->text().trimmed().toStdString(),
                  firstNameEdit->text().trimmed().toStdString(),
                  phoneEdit->text().trimmed().toStdString(),
                  emailEdit->text().trimmed().toStdString());
}
