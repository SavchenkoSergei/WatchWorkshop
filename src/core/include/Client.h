#pragma once
#include <compare>
#include <iosfwd>
#include <string>

class Client {
public:
    Client() = default;
    Client(int id, std::string lastName, std::string firstName, std::string phone, std::string email);

    int getId() const { return id; }
    void setId(int newId) { id = newId; }
    const std::string& getLastName() const { return lastName; }
    const std::string& getFirstName() const { return firstName; }
    const std::string& getPhone() const { return phone; }
    const std::string& getEmail() const { return email; }

    bool operator==(const Client& other) const;
    std::strong_ordering operator<=>(const Client& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Client& c);
    friend std::istream& operator>>(std::istream& is, Client& c);

private:
    int id = 0;
    std::string lastName, firstName, phone, email;
};
