#pragma once
#include "Service.h"


class QuartzRepairService : public Service {
public:
    QuartzRepairService(std::string name, double baseCost, int complexity);
    double calculateCost() const override;
    int calculateDuration() const override;
    void write(std::ostream& os) const override;
};
