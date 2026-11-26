#include "OrderDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>

#include "Exceptions.h"

OrderDialog::OrderDialog(const Choices& clients, const Choices& watches, const Choices& masters,
                         bool serviceOnly, QWidget* parent, int preselectedClientId,
                         int preselectedWatchId)
    : QDialog(parent), allWatches(watches), serviceOnly(serviceOnly) {
    setWindowTitle(serviceOnly ? "Добавление услуги" : "Новый заказ");
    setMinimumWidth(460);
    form = new QFormLayout(this);

    serviceBox = new QComboBox(this);
    brandLabel = new QLabel(this);
    priceLabel = new QLabel(this);                            // цена работы из каталога услуг
    complexitySpin = new QSpinBox(this);
    complexitySpin->setRange(1, 5);
    disassemblySpin = new QSpinBox(this);
    disassemblySpin->setRange(1, 3);
    complicationSpin = new QSpinBox(this);
    complicationSpin->setRange(1, 10);
    previewLabel = new QLabel(this);
    previewLabel->setStyleSheet("font-weight: bold;");

    if (!serviceOnly) {
        clientBox = new QComboBox(this);
        for (const Choice& c : clients) clientBox->addItem(c.title, c.id);
        watchBox = new QComboBox(this);
        masterBox = new QComboBox(this);
        masterBox->addItem("— не назначен —", 0);
        for (const Choice& m : masters) masterBox->addItem(m.title, m.id);
        form->addRow("Клиент:", clientBox);
        form->addRow("Часы:", watchBox);
        form->addRow("Мастер:", masterBox);
    }
    form->addRow("Услуга:", serviceBox);
    form->addRow("Класс марки:", brandLabel);
    form->addRow("Цена работы, BYN:", priceLabel);
    form->addRow("Сложность (1–5):", complexitySpin);
    form->addRow("Уровень разборки (1–3):", disassemblySpin);
    form->addRow("Доп. функции (календарь, фаза луны и т. п.):", complicationSpin);
    form->addRow("Стоимость услуги и срок:", previewLabel);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &OrderDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &OrderDialog::reject);

    connect(serviceBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this] { updateServiceFields(); });
    connect(complexitySpin, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updatePreview(); });
    connect(disassemblySpin, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updatePreview(); });
    connect(complicationSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updatePreview(); });

    if (!serviceOnly) {
        connect(clientBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
                [this] { refillWatches(); });
        connect(watchBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
                [this] { refillServices(); });
        // клиент и часы, выбранные заранее (например, через контекстное меню)
        if (preselectedClientId != 0) {
            int index = clientBox->findData(preselectedClientId);
            if (index >= 0) clientBox->setCurrentIndex(index);
        }
        refillWatches();
        if (preselectedWatchId != 0) {
            int index = watchBox->findData(preselectedWatchId);
            if (index >= 0) watchBox->setCurrentIndex(index);
        }
    }
    refillServices();
}

const Choice* OrderDialog::currentWatch() const {
    if (serviceOnly) return allWatches.isEmpty() ? nullptr : &allWatches.first();
    int id = watchBox->currentData().toInt();
    for (const Choice& w : allWatches)
        if (w.id == id) return &w;
    return nullptr;
}

const ServiceTemplate* OrderDialog::currentTemplate() const {
    int index = serviceBox->currentData().toInt();
    if (serviceBox->count() == 0 || index < 0 || index >= static_cast<int>(offered.size())) return nullptr;
    return offered[static_cast<std::size_t>(index)];
}

void OrderDialog::refillWatches() {
    watchBox->blockSignals(true);
    watchBox->clear();
    int owner = clientBox->currentData().toInt();
    for (const Choice& w : allWatches)
        if (w.parentId == owner) watchBox->addItem(w.title, w.id);
    watchBox->blockSignals(false);
    refillServices();
}

// Набор услуг зависит от типа механизма часов (кварцевый, механический, хронограф).
void OrderDialog::refillServices() {
    serviceBox->blockSignals(true);
    serviceBox->clear();
    offered.clear();
    if (const Choice* watch = currentWatch())
        offered = ServiceCatalog::forMechanism(watch->mechanism.toStdString());
    for (std::size_t i = 0; i < offered.size(); ++i)
        serviceBox->addItem(QString::fromStdString(offered[i]->name), static_cast<int>(i));
    serviceBox->blockSignals(false);
    updateServiceFields();
}

// Показывает только нужное услуге поле и пересчитывает цену работы с учётом класса марки.
void OrderDialog::updateServiceFields() {
    const ServiceTemplate* t = currentTemplate();
    ServiceExtra extra = t ? t->extra : ServiceExtra::None;
    auto show = [this](QWidget* field, bool visible) {
        form->labelForField(field)->setVisible(visible);
        field->setVisible(visible);
    };
    show(disassemblySpin, extra == ServiceExtra::Disassembly);
    show(complicationSpin, extra == ServiceExtra::Complications);

    BrandInfo brand{"обычная", 1.0};
    if (const Choice* watch = currentWatch()) brand = ServiceCatalog::brandInfo(watch->brand.toStdString());
    brandLabel->setText(QString::fromStdString(brand.title) + " (×" + QString::number(brand.factor, 'f', 2) + ")");
    priceLabel->setText(t ? QString::number(t->basePrice * brand.factor, 'f', 2) +
                            "  (каталог услуг × класс марки)" : QString("—"));
    updatePreview();
}

void OrderDialog::updatePreview() {
    try {
        std::shared_ptr<Service> s = getService();
        previewLabel->setText(QString("%1 BYN, срок %2 раб. дн.")
                                  .arg(s->calculateCost(), 0, 'f', 2)
                                  .arg(s->calculateDuration()));
    } catch (const WorkshopException& e) {
        previewLabel->setText(QString::fromUtf8(e.what()));
    }
}

void OrderDialog::validate() const {
    if (!serviceOnly) {
        if (clientBox->count() == 0) throw InvalidInputException("Клиент (сначала добавьте клиента)");
        if (watchBox->count() == 0) throw InvalidInputException("Часы (у клиента нет принятых часов)");
    }
    if (!currentTemplate())
        throw InvalidInputException("Услуга (для этого типа механизма нет услуг в прайс-листе)");
}

void OrderDialog::accept() {
    try {
        validate();
        getService();            // создание услуги проверяет параметры (может выбросить исключение)
        QDialog::accept();
    } catch (const InvalidInputException& e) {
        QMessageBox::warning(this, "Ошибка ввода", QString::fromUtf8(e.what()));
    }
}

int OrderDialog::getClientId() const { return clientBox ? clientBox->currentData().toInt() : 0; }
int OrderDialog::getWatchId() const {
    if (serviceOnly) return allWatches.isEmpty() ? 0 : allWatches.first().id;
    return watchBox->currentData().toInt();
}
int OrderDialog::getMasterId() const { return masterBox ? masterBox->currentData().toInt() : 0; }

std::shared_ptr<Service> OrderDialog::getService() const {
    const ServiceTemplate* t = currentTemplate();
    if (!t) throw InvalidInputException("Услуга");
    BrandInfo brand{"обычная", 1.0};
    if (const Choice* watch = currentWatch()) brand = ServiceCatalog::brandInfo(watch->brand.toStdString());
    int extraValue = 0;
    switch (t->extra) {
        case ServiceExtra::Disassembly: extraValue = disassemblySpin->value(); break;
        case ServiceExtra::Complications: extraValue = complicationSpin->value(); break;
        case ServiceExtra::None: break;
    }
    return ServiceCatalog::create(*t, complexitySpin->value(), extraValue, brand.factor);
}
