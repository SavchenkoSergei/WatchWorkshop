#pragma once
#include <memory>
#include <vector>

#include <QDialog>

#include "DialogTypes.h"
#include "Service.h"
#include "ServiceCatalog.h"

class QComboBox;
class QFormLayout;
class QLabel;
class QSpinBox;

// Диалог оформления заказа. Набор услуг зависит от типа механизма выбранных часов,
// цена работы - от класса марки. Элементы питания и другие детали мастер добавляет в заказ отдельно.
// В режиме serviceOnly показываются только поля услуги для часов существующего заказа
// (в списке watches передаётся одна запись).
class OrderDialog : public QDialog {
public:
    OrderDialog(const Choices& clients, const Choices& watches, const Choices& masters,
                bool serviceOnly = false, QWidget* parent = nullptr,
                int preselectedClientId = 0, int preselectedWatchId = 0);

    int getClientId() const;
    int getWatchId() const;
    int getMasterId() const;                       // 0 - мастер не назначен
    std::shared_ptr<Service> getService() const;   // услуга с введёнными параметрами
    void accept() override;

private:
    void validate() const;
    const Choice* currentWatch() const;
    const ServiceTemplate* currentTemplate() const;
    void refillWatches();        // показывает только часы выбранного клиента
    void refillServices();       // услуги, подходящие типу механизма выбранных часов
    void updateServiceFields();  // показывает нужные поля и пересчитывает цену работы
    void updatePreview();        // пересчитывает стоимость и срок

    Choices allWatches;
    bool serviceOnly;
    std::vector<const ServiceTemplate*> offered;
    QFormLayout* form;
    QComboBox* clientBox = nullptr;
    QComboBox* watchBox = nullptr;
    QComboBox* masterBox = nullptr;
    QComboBox* serviceBox;
    QLabel* brandLabel;
    QLabel* priceLabel;
    QSpinBox* complexitySpin;
    QSpinBox* disassemblySpin;
    QSpinBox* complicationSpin;
    QLabel* previewLabel;
};
