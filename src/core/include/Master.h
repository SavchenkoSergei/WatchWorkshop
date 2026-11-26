#pragma once
#include <compare>
#include <iosfwd>
#include <string>

class Master {
public:
    Master() = default;
    Master(int id, std::string lastName, std::string firstName, std::string specialization,
           std::string phone);

    int getId() const { return id; }
    void setId(int newId) { id = newId; }
    const std::string& getLastName() const { return lastName; }
    const std::string& getFirstName() const { return firstName; }
    const std::string& getSpecialization() const { return specialization; }
    const std::string& getPhone() const { return phone; }

    bool operator==(const Master& other) const;
    std::strong_ordering operator<=>(const Master& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Master& m);
    friend std::istream& operator>>(std::istream& is, Master& m);

private:
    int id = 0;
    std::string lastName, firstName, specialization, phone;
};
