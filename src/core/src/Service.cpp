#include "Service.h"

#include <iomanip>
#include <iostream>
#include <utility>

#include "ChronographRepairService.h"
#include "Exceptions.h"
#include "MechanicalRepairService.h"
#include "QuartzRepairService.h"

Service::Service(std::string name, double baseCost, int complexity)
    : name_(std::move(name)), baseCost_(baseCost), complexity_(complexity) {
    if (name_.empty()) throw InvalidInputException("name");
    if (baseCost_ < 0) throw InvalidInputException("baseCost");
    if (complexity < 1 || complexity > 5) throw InvalidInputException("complexity");
}

void Service::writeCommon(std::ostream& os, const char* typeTag) const {
    os << typeTag << ' ' << std::quoted(name_) << ' '
        << std::setprecision(10) << baseCost_ << ' ' << complexity_ << ' ';
}

std::shared_ptr<Service> Service::read(std::istream& is) {
    std::string tag, name;
    double base = 0, extra = 0;
    int cx = 0;
    if (!(is >> tag >> std::quoted(name) >> base >> cx >> extra)) return nullptr;
    try {
        if (tag == "quartz")     return std::make_shared<QuartzRepairService>(name, base, cx);
        if (tag == "mechanical") return std::make_shared<MechanicalRepairService>(name, base, cx, static_cast<int>(extra));
        if (tag == "chronograph") return std::make_shared<ChronographRepairService>(name, base, cx, static_cast<int>(extra));
    }
    catch (const InvalidInputException&) {
        return nullptr;
    }
    return nullptr;
}