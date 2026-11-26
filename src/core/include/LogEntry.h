#pragma once
#include <iosfwd>
#include <string>

#include "Account.h"

class LogEntry {
public:
    LogEntry() = default;
    LogEntry(int id, std::string time, std::string login, Role role, std::string action,
             std::string details);

    int getId() const { return id; }
    const std::string& getTime() const { return time; }
    const std::string& getLogin() const { return login; }
    Role getRole() const { return role; }
    const std::string& getAction() const { return action; }
    const std::string& getDetails() const { return details; }

    bool operator==(const LogEntry& other) const { return id == other.id; }
    friend std::ostream& operator<<(std::ostream& os, const LogEntry& e);
    friend std::istream& operator>>(std::istream& is, LogEntry& e);

private:
    int id = 0;
    std::string time, login;
    Role role = Role::Admin;
    std::string action, details;
};
