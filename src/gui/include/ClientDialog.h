#pragma once
#include <QDialog>

#include "Client.h"

class QLineEdit;

class ClientDialog : public QDialog {
public:
    // existing != nullptr - режим редактирования (поля заполняются из объекта)
    explicit ClientDialog(QWidget* parent = nullptr, const Client* existing = nullptr);
    Client getClient() const;
    void accept() override;

private:
    void validate() const;

    QLineEdit* lastNameEdit;
    QLineEdit* firstNameEdit;
    QLineEdit* phoneEdit;
    QLineEdit* emailEdit;
};
