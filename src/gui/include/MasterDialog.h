#pragma once
#include <QDialog>

#include "Master.h"

class QLineEdit;

class MasterDialog : public QDialog {
public:
    explicit MasterDialog(QWidget* parent = nullptr, const Master* existing = nullptr);
    Master getMaster() const;
    void accept() override;

private:
    void validate() const;

    QLineEdit* lastNameEdit;
    QLineEdit* firstNameEdit;
    QLineEdit* specializationEdit;
    QLineEdit* phoneEdit;
};
