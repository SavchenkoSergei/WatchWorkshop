#pragma once
#include <QDialog>

class QLineEdit;

class LoginDialog : public QDialog {
public:
    explicit LoginDialog(QWidget* parent = nullptr);
    QString getLogin() const;
    QString getPassword() const;
    bool isGuest() const { return guest; }
    void accept() override;

private:
    void onGuest();

    QLineEdit* loginEdit;
    QLineEdit* passwordEdit;
    bool guest = false;
};
