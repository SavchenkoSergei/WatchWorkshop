#pragma once
#include "Service.h"


class MechanicalRepairService : public Service {
public:
    MechanicalRepairService(std::string name, double baseCost, int complexity, int disassemblyLevel);
    double calculateCost() const override;
    int calculateDuration() const override;
    void write(std::ostream& os) const override;
    int getDisassemblyLevel() const { return disassemblyLevel; }
private:
    static constexpr double levelPrice = 25.0; 
    int disassemblyLevel;
};
