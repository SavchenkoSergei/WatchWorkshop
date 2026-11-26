#include "DetailsDialog.h"

#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QVBoxLayout>

DetailsDialog::DetailsDialog(const QString& title, const QString& html, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(title);
    resize(560, 460);
    auto* browser = new QTextBrowser(this);
    browser->setHtml(html);
    browser->setOpenLinks(false);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(browser);
    layout->addWidget(buttons);
}
