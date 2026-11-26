#include "LoginDialog.h"

#include <memory>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Часовая мастерская: вход");
    loginEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);

    auto form = std::make_unique<QFormLayout>();
    form->addRow("Логин:", loginEdit);
    form->addRow("Пароль:", passwordEdit);

    auto* enter = new QPushButton("Войти", this);
    enter->setDefault(true);
    auto* guestButton = new QPushButton("Гость (только просмотр)", this);
    auto* quit = new QPushButton("Выход", this);

    auto buttons = std::make_unique<QHBoxLayout>();
    buttons->addWidget(enter);
    buttons->addWidget(guestButton);
    buttons->addWidget(quit);

    auto layout = std::make_unique<QVBoxLayout>(this);
    layout->addWidget(new QLabel("Вход для сотрудников мастерской", this));
    layout->addLayout(form.release());
    layout->addLayout(buttons.release());
    layout.release();

    connect(enter, &QPushButton::clicked, this, &LoginDialog::accept);
    connect(guestButton, &QPushButton::clicked, this, &LoginDialog::onGuest);
    connect(quit, &QPushButton::clicked, this, &LoginDialog::reject);
}