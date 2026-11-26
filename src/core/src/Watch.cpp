#include "Watch.h"

#include <iomanip>
#include <iostream>

#include "Client.h"
#include "Workshop.h"

Watch::Watch(int id, int clientId, std::string brand, std::string model, std::string serialNumber,
             std::string mechanismType, std::string defectDescription)
    : id(id), clientId(clientId), brand(std::move(brand)), model(std::move(model)),
      serialNumber(std::move(serialNumber)), mechanismType(std::move(mechanismType)),
      defectDescription(std::move(defectDescription)) {}

const std::vector<std::string>& Watch::mechanismTypes() {
    static const std::vector<std::string> types = {"кварцевый", "механический", "хронограф"};
    return types;
}

void Watch::setClient(Client* c) {
    client = c;
    clientId = c ? c->getId() : 0;
}

void Watch::resolveLinks(Workshop& w) { client = &w.clients.findById(clientId); }

bool Watch::operator==(const Watch& other) const { return id == other.id; }

std::strong_ordering Watch::operator<=>(const Watch& other) const {
    if (auto c = brand <=> other.brand; c != 0) return c;
    if (auto c = model <=> other.model; c != 0) return c;
    return serialNumber <=> other.serialNumber;
}

std::ostream& operator<<(std::ostream& os, const Watch& w) {
    return os << w.id << ' ' << w.clientId << ' ' << std::quoted(w.brand) << ' '
              << std::quoted(w.model) << ' ' << std::quoted(w.serialNumber) << ' '
              << std::quoted(w.mechanismType) << ' ' << std::quoted(w.defectDescription) << '\n';
}

std::istream& operator>>(std::istream& is, Watch& w) {
    return is >> w.id >> w.clientId >> std::quoted(w.brand) >> std::quoted(w.model)
              >> std::quoted(w.serialNumber) >> std::quoted(w.mechanismType)
              >> std::quoted(w.defectDescription);
}
