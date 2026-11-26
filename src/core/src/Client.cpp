#include "Client.h"

#include <iomanip>
#include <iostream>

Client::Client(int id, std::string lastName, std::string firstName, std::string phone, std::string email)
    : id(id), lastName(std::move(lastName)), firstName(std::move(firstName)),
      phone(std::move(phone)), email(std::move(email)) {}

bool Client::operator==(const Client& other) const { return id == other.id; }

std::strong_ordering Client::operator<=>(const Client& other) const {
    if (auto c = lastName <=> other.lastName; c != 0) return c;
    if (auto c = firstName <=> other.firstName; c != 0) return c;
    return id <=> other.id;
}

std::ostream& operator<<(std::ostream& os, const Client& c) {
    return os << c.id << ' ' << std::quoted(c.lastName) << ' ' << std::quoted(c.firstName) << ' '
              << std::quoted(c.phone) << ' ' << std::quoted(c.email) << '\n';
}

std::istream& operator>>(std::istream& is, Client& c) {
    return is >> c.id >> std::quoted(c.lastName) >> std::quoted(c.firstName)
              >> std::quoted(c.phone) >> std::quoted(c.email);
}
