#include "Master.h"

#include <iomanip>
#include <iostream>

Master::Master(int id, std::string lastName, std::string firstName, std::string specialization,
               std::string phone)
    : id(id), lastName(std::move(lastName)), firstName(std::move(firstName)),
      specialization(std::move(specialization)), phone(std::move(phone)) {}

bool Master::operator==(const Master& other) const { return id == other.id; }

std::strong_ordering Master::operator<=>(const Master& other) const {
    if (auto c = lastName <=> other.lastName; c != 0) return c;
    if (auto c = firstName <=> other.firstName; c != 0) return c;
    return id <=> other.id;
}

std::ostream& operator<<(std::ostream& os, const Master& m) {
    return os << m.id << ' ' << std::quoted(m.lastName) << ' ' << std::quoted(m.firstName) << ' '
              << std::quoted(m.specialization) << ' ' << std::quoted(m.phone) << '\n';
}

std::istream& operator>>(std::istream& is, Master& m) {
    return is >> m.id >> std::quoted(m.lastName) >> std::quoted(m.firstName)
              >> std::quoted(m.specialization) >> std::quoted(m.phone);
}
