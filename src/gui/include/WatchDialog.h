#pragma once
#include <QDialog>

#include "DialogTypes.h"
#include "Watch.h"

class QComboBox;
class QLineEdit;
class QPlainTextEdit;

class WatchDialog : public QDialog {
public:
    explicit WatchDialog(const Choices& clients, QWidget* parent = nullptr,
        const Watch* existing = nullptr, int preselectedClientId = 0);
    Watch getWatch() const;
    void accept() override;

private:
    void validate() const;

    QComboBox* clientBox;
    QLineEdit* brandEdit;
    QLineEdit* modelEdit;
    QLineEdit* serialEdit;
    QComboBox* mechanismBox;
    QPlainTextEdit* defectEdit;
};