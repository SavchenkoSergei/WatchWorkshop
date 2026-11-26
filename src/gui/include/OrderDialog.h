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

class OrderDialog : public QDialog {
public:
    OrderDialog(const Choices& clients, const Choices& watches, const Choices& masters,
                bool serviceOnly = false, QWidget* parent = nullptr,
                int preselectedClientId = 0, int preselectedWatchId = 0);

    int getClientId() const;
    int getWatchId() const;
    int getMasterId() const;                    
    std::shared_ptr<Service> getService() const;   
    void accept() override;

private:
    void validate() const;
    const Choice* currentWatch() const;
    const ServiceTemplate* currentTemplate() const;
    void refillWatches();       
    void refillServices();      
    void updateServiceFields(); 
    void updatePreview();       

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
