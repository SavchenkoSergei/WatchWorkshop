#include "MechanicalRepairService.h"

#include <iostream>

#include "Exceptions.h"

MechanicalRepairService::MechanicalRepairService(std::string name, double baseCost, int complexity,
                                                 int disassemblyLevel)
    : Service(std::move(name), baseCost, complexity), disassemblyLevel(disassemblyLevel) {
    if (disassemblyLevel < 0 || disassemblyLevel > 3) throw InvalidInputException("disassemblyLevel");
}

double MechanicalRepairService::calculateCost() const {
    return baseWithComplexity() + levelPrice * disassemblyLevel;
}

int MechanicalRepairService::calculateDuration() const { return complexity + disassemblyLevel; }

void MechanicalRepairService::write(std::ostream& os) const {
    writeCommon(os, "mechanical");
    os << disassemblyLevel << ' ';
}
