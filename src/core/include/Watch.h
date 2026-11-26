#pragma once
#include <compare>
#include <iosfwd>
#include <string>
#include <vector>

class Client;
class Workshop;

class Watch {
public:
    Watch() = default;
    Watch(int id, int clientId, std::string brand, std::string model, std::string serialNumber,
          std::string mechanismType, std::string defectDescription);

    int getId() const { return id; }
    void setId(int newId) { id = newId; }
    int getClientId() const { return clientId; }
    Client* getClient() const { return client; }
    void setClient(Client* c);
    const std::string& getBrand() const { return brand; }
    const std::string& getModel() const { return model; }
    const std::string& getSerialNumber() const { return serialNumber; }
    const std::string& getMechanismType() const { return mechanismType; }
    const std::string& getDefectDescription() const { return defectDescription; }

    static const std::vector<std::string>& mechanismTypes();

    void resolveLinks(Workshop& w);

    bool operator==(const Watch& other) const;
    std::strong_ordering operator<=>(const Watch& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Watch& w);
    friend std::istream& operator>>(std::istream& is, Watch& w);

private:
    int id = 0;
    int clientId = 0;
    Client* client = nullptr;
    std::string brand, model, serialNumber, mechanismType, defectDescription;
};
