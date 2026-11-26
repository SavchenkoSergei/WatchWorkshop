#include "ChronographRepairService.h"

#include <iostream>

#include "Exceptions.h"

ChronographRepairService::ChronographRepairService(std::string name, double baseCost, int complexity,
                                                   int complicationCount)
    : Service(std::move(name), baseCost, complexity), complicationCount(complicationCount) {
    if (complicationCount < 0 || complicationCount > 10) throw InvalidInputException("complicationCount");
}

double ChronographRepairService::calculateCost() const {
    return baseWithComplexity() + complicationPrice * complicationCount;
}

int ChronographRepairService::calculateDuration() const { return getComplexity() + complicationCount; }

void ChronographRepairService::write(std::ostream& os) const {
    writeCommon(os, "chronograph");
    os << complicationCount << ' ';
}
