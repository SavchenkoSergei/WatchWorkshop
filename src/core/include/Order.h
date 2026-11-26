#pragma once
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "OrderStatus.h"
#include "Service.h"

class Client;
class Watch;
class Master;
class Part;
class Workshop;

// Идентификаторы связанных объектов, прочитанные из файла.
struct OrderLinks {
    int clientId = 0;
    int watchId = 0;
    int masterId = 0;
    std::vector<int> partIds;
};

// Заказ-наряд. Указатели на клиента, часы, мастера и детали не владеют объектами
// (ими владеют репозитории). Услугами владеет заказ через shared_ptr.
class Order {
public:
    Order() = default;
    Order(int id, Client* client, Watch* watch, Master* master,
          std::shared_ptr<Service> service, std::string acceptDate);

    int getId() const { return id; }
    Client* getClient() const { return client; }
    Watch* getWatch() const { return watch; }
    Master* getMaster() const { return master; }
    const std::vector<std::shared_ptr<Service>>& getServices() const { return services; }
    OrderStatus getStatus() const { return status; }
    const std::string& getAcceptDate() const { return acceptDate; }
    const std::string& getIssueDate() const { return issueDate; }       // пусто, пока заказ не выдан
    const std::vector<Part*>& getUsedParts() const { return usedParts; }

    void setStatus(OrderStatus newStatus) { status = newStatus; }
    void setIssueDate(std::string date) { issueDate = std::move(date); }
    void assignMaster(Master* m) { master = m; }
    void addService(std::shared_ptr<Service> service);
    void removeService(std::size_t index);   // в заказе должна остаться хотя бы одна услуга
    void addUsedPart(Part* part) { usedParts.push_back(part); }
    void removeUsedPart(Part* part);       // убирает одну единицу детали

    // Наибольшая сложность среди услуг; заказ со сложностью от 4 считается сложным:
    // мастер не может взять его сам, его назначает ресепшен.
    int maxComplexity() const;
    bool isComplex() const { return maxComplexity() >= complexThreshold; }
    static constexpr int complexThreshold = 4;

    double totalCost() const;              // C = сумма C_i + сумма P_j
    int deadlineDays() const;              // T = сумма T_i
    std::string plannedDate() const;       // D2 = D1 + T (в рабочих днях)
    int daysLeft(const std::string& today) const;   // дней до плановой даты (отрицательное - просрочен)

    void resolveLinks(Workshop& w);

    bool operator==(const Order& other) const { return id == other.id; }
    friend std::ostream& operator<<(std::ostream& os, const Order& o);
    friend std::istream& operator>>(std::istream& is, Order& o);

private:
    int id = 0;
    Client* client = nullptr;
    Watch* watch = nullptr;
    Master* master = nullptr;
    std::vector<std::shared_ptr<Service>> services;
    OrderStatus status = OrderStatus::Accepted;
    std::string acceptDate;
    std::string issueDate;
    OrderLinks links;
    std::vector<Part*> usedParts;
};
