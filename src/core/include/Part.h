#pragma once
#include <compare>
#include <iosfwd>
#include <string>

class Part {
public:
    Part() = default;
    Part(int id, std::string name, std::string category, int quantity, double price);

    int getId() const { return id; }
    void setId(int newId) { id = newId; }
    const std::string& getName() const { return name; }
    const std::string& getCategory() const { return category; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }

    void addStock(int qty);   
    void writeOff(int qty);   

    bool operator==(const Part& other) const;
    std::strong_ordering operator<=>(const Part& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Part& p);
    friend std::istream& operator>>(std::istream& is, Part& p);

private:
    int id = 0;
    std::string name, category;
    int quantity = 0;
    double price = 0;
};
