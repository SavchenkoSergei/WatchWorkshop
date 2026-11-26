#pragma once
#include <iosfwd>
#include <string>

enum class Role { Guest = 0, Master, Reception, Admin };

inline std::string roleToString(Role r) {
    switch (r) {
        case Role::Guest:     return "Гость";
        case Role::Master:    return "Мастер";
        case Role::Reception: return "Ресепшен";
        case Role::Admin:     return "Администратор";
    }
    return "?";
}

class Account {
public:
    Account() = default;
    Account(int id, std::string login, std::string passwordHash, Role role, int masterId);

    int getId() const { return id; }
    void setId(int newId) { id = newId; }
    const std::string& getLogin() const { return login; }
    Role getRole() const { return role; }
    int getMasterId() const { return masterId; }     
    void setPasswordHash(std::string hash) { passwordHash = std::move(hash); }

    bool checkPassword(const std::string& password) const;
    static std::string hashPassword(const std::string& login, const std::string& password);

    bool operator==(const Account& other) const { return id == other.id; }
    friend std::ostream& operator<<(std::ostream& os, const Account& a);
    friend std::istream& operator>>(std::istream& is, Account& a);

private:
    int id = 0;
    std::string login, passwordHash;
    Role role = Role::Master;
    int masterId = 0;
};
