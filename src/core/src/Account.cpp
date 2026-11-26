#include "Account.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>

Account::Account(int id, std::string login, std::string passwordHash, Role role, int masterId)
    : id(id), login(std::move(login)), passwordHash(std::move(passwordHash)),
      role(role), masterId(masterId) {}

std::string Account::hashPassword(const std::string& login, const std::string& password) {
    std::uint64_t h = 1469598103934665603ULL;                
    const std::string data = "ww1:" + login + ":" + password;  
    for (int round = 0; round < 1000; ++round)
        for (unsigned char c : data) {
            h ^= c;
            h *= 1099511628211ULL;
        }
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << h;
    return out.str();
}

bool Account::checkPassword(const std::string& password) const {
    return passwordHash == hashPassword(login, password);
}

std::ostream& operator<<(std::ostream& os, const Account& a) {
    return os << a.id << ' ' << std::quoted(a.login) << ' ' << a.passwordHash << ' '
              << static_cast<int>(a.role) << ' ' << a.masterId << '\n';
}

std::istream& operator>>(std::istream& is, Account& a) {
    int r = 0;
    is >> a.id >> std::quoted(a.login) >> a.passwordHash >> r >> a.masterId;
    if (r < 1 || r > 3) is.setstate(std::ios::failbit);      
    a.role = static_cast<Role>(r);
    return is;
}
