#pragma once
#include "Service.h"

// Ремонт кварцевых часов.
//   X = 0, Y = 0. Элемент питания в услугу не входит: мастер добавляет его в заказ
//   как обычную деталь, и её цена прибавляется к стоимости заказа.
class QuartzRepairService : public Service {
public:
    QuartzRepairService(std::string name, double baseCost, int complexity);
    double calculateCost() const override;
    int calculateDuration() const override;
    void write(std::ostream& os) const override;
};
