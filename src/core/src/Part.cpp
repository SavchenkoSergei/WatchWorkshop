#include "Part.h"

#include <iomanip>
#include <iostream>

#include "Exceptions.h"

Part::Part(int id, std::string name, std::string category, int quantity, double price)
    : id(id), name(std::move(name)), category(std::move(category)), quantity(quantity), price(price) {}

void Part::addStock(int qty) {
    if (qty <= 0) throw InvalidInputException("qty");
    quantity += qty;
}

void Part::writeOff(int qty) {
    if (qty <= 0) throw InvalidInputException("qty");
    if (qty > quantity) throw OutOfStockException(name);
    quantity -= qty;
}

bool Part::operator==(const Part& other) const { return id == other.id; }

std::strong_ordering Part::operator<=>(const Part& other) const {
    if (auto c = name <=> other.name; c != 0) return c;
    return category <=> other.category;
}

std::ostream& operator<<(std::ostream& os, const Part& p) {
    return os << p.id << ' ' << std::quoted(p.name) << ' ' << std::quoted(p.category) << ' '
              << p.quantity << ' ' << std::setprecision(10) << p.price << '\n';
}

std::istream& operator>>(std::istream& is, Part& p) {
    return is >> p.id >> std::quoted(p.name) >> std::quoted(p.category) >> p.quantity >> p.price;
}
