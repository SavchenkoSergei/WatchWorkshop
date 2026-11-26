#pragma once
#include "Service.h"

// Ремонт механических часов.
//   X = M * R  (R - уровень разборки 0..3, 0 - без разборки; M - надбавка за уровень), Y = R
class MechanicalRepairService : public Service {
public:
    MechanicalRepairService(std::string name, double baseCost, int complexity, int disassemblyLevel);
    double calculateCost() const override;
    int calculateDuration() const override;
    void write(std::ostream& os) const override;
    int getDisassemblyLevel() const { return disassemblyLevel; }
private:
    static constexpr double levelPrice = 25.0;    // M, BYN
    int disassemblyLevel;
};
