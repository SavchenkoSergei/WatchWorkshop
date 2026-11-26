#include "QuartzRepairService.h"

#include <iostream>

QuartzRepairService::QuartzRepairService(std::string name, double baseCost, int complexity)
    : Service(std::move(name), baseCost, complexity) {}

double QuartzRepairService::calculateCost() const { return baseWithComplexity(); }

int QuartzRepairService::calculateDuration() const { return getComplexity(); }

void QuartzRepairService::write(std::ostream& os) const {
    writeCommon(os, "quartz");
    os << 0 << ' ';                                  
}
