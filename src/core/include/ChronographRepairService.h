#pragma once
#include "Service.h"

class ChronographRepairService : public Service {
public:
    ChronographRepairService(std::string name, double baseCost, int complexity, int complicationCount);
    double calculateCost() const override;
    int calculateDuration() const override;
    void write(std::ostream& os) const override;
    int getComplicationCount() const { return complicationCount; }
private:
    static constexpr double complicationPrice = 40.0;   
    int complicationCount;
};
