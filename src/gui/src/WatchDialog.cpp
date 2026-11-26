#include "WatchDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>

#include "Exceptions.h"
#include "Validation.h"

WatchDialog::WatchDialog(const Choices& clients, QWidget* parent, const Watch* existing,
                         int preselectedClientId)
    : QDialog(parent) {
    setWindowTitle(existing ? "Редактирование часов" : "Приём часов в ремонт");
    clientBox = new QComboBox(this);
    for (const Choice& c : clients) clientBox->addItem(c.title, c.id);
    if (!existing && preselectedClientId != 0) {
        int index = clientBox->findData(preselectedClientId);
        if (index >= 0) clientBox->setCurrentIndex(index);
    }
    brandEdit = new QLineEdit(this);
    modelEdit = new QLineEdit(this);
    serialEdit = new QLineEdit(this);
    mechanismBox = new QComboBox(this);
    for (const std::string& type : Watch::mechanismTypes())      // выбор только из списка
        mechanismBox->addItem(QString::fromStdString(type));
    defectEdit = new QPlainTextEdit(this);
    defectEdit->setFixedHeight(70);

    if (existing) {
        int index = clientBox->findData(existing->getClientId());
        if (index >= 0) clientBox->setCurrentIndex(index);
        brandEdit->setText(QString::fromStdString(existing->getBrand()));
        modelEdit->setText(QString::fromStdString(existing->getModel()));
        serialEdit->setText(QString::fromStdString(existing->getSerialNumber()));
        mechanismBox->setCurrentText(QString::fromStdString(existing->getMechanismType()));
        defectEdit->setPlainText(QString::fromStdString(existing->getDefectDescription()));
    }

    auto* form = new QFormLayout(this);
    form->addRow("Владелец:", clientBox);
    form->addRow("Марка:", brandEdit);
    form->addRow("Модель:", modelEdit);
    form->addRow("Серийный номер:", serialEdit);
    form->addRow("Тип механизма:", mechanismBox);
    form->addRow("Неисправность:", defectEdit);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &WatchDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &WatchDialog::reject);
}

void WatchDialog::validate() const {
    using namespace validation;
    if (clientBox->count() == 0) throw InvalidInputException("Владелец (сначала добавьте клиента)");
    if (!isValidText(brandEdit->text().trimmed().toStdString())) throw InvalidInputException("Марка");
    if (!isValidText(modelEdit->text().trimmed().toStdString())) throw InvalidInputException("Модель");
    if (!isValidSerial(serialEdit->text().trimmed().toStdString()))
        throw InvalidInputException("Серийный номер (латинские буквы, цифры и знаки - _ / .)");
}

void WatchDialog::accept() {
    try {
        validate();
        QDialog::accept();
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Ошибка ввода", QString::fromUtf8(e.what()));
    }
}

Watch WatchDialog::getWatch() const {
    return Watch(0, clientBox->currentData().toInt(), brandEdit->text().trimmed().toStdString(),
                 modelEdit->text().trimmed().toStdString(),
                 serialEdit->text().trimmed().toStdString(),
                 mechanismBox->currentText().trimmed().toStdString(),
                 defectEdit->toPlainText().trimmed().toStdString());
}
