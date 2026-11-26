#include "LogEntry.h"

#include <iomanip>
#include <iostream>

LogEntry::LogEntry(int id, std::string time, std::string login, Role role, std::string action,
                   std::string details)
    : id(id), time(std::move(time)), login(std::move(login)), role(role),
      action(std::move(action)), details(std::move(details)) {}

std::ostream& operator<<(std::ostream& os, const LogEntry& e) {
    return os << e.id << ' ' << std::quoted(e.time) << ' ' << std::quoted(e.login) << ' '
              << static_cast<int>(e.role) << ' ' << std::quoted(e.action) << ' '
              << std::quoted(e.details) << '\n';
}

std::istream& operator>>(std::istream& is, LogEntry& e) {
    int r = 0;
    is >> e.id >> std::quoted(e.time) >> std::quoted(e.login) >> r >> std::quoted(e.action)
       >> std::quoted(e.details);
    if (r < 0 || r > 3) is.setstate(std::ios::failbit);
    e.role = static_cast<Role>(r);
    return is;
}
