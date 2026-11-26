#include "Order.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "Client.h"
#include "Dates.h"
#include "Exceptions.h"
#include "Master.h"
#include "Part.h"
#include "Watch.h"
#include "Workshop.h"

Order::Order(int id, Client* client, Watch* watch, Master* master,
             std::shared_ptr<Service> service, std::string acceptDate)
    : id(id), client(client), watch(watch), master(master), acceptDate(std::move(acceptDate)) {
    addService(std::move(service));
}

void Order::addService(std::shared_ptr<Service> service) {
    if (!service) throw InvalidInputException("service");
    services.push_back(std::move(service));
}

void Order::removeService(std::size_t index) {
    if (index >= services.size()) throw ObjectNotFoundException(static_cast<int>(index));
    if (services.size() == 1) throw InvalidInputException("Нельзя удалить единственную услугу заказа");
    services.erase(services.begin() + static_cast<std::ptrdiff_t>(index));
}

void Order::removeUsedPart(Part* part) {
    auto it = std::find(usedParts.begin(), usedParts.end(), part);
    if (it == usedParts.end()) throw ObjectNotFoundException(part ? part->getId() : 0);
    usedParts.erase(it);
}

int Order::maxComplexity() const {
    int result = 0;
    for (const auto& s : services) result = std::max(result, s->getComplexity());
    return result;
}

double Order::totalCost() const {
    double sum = 0;
    for (const auto& s : services) sum += s->calculateCost();   // виртуальный вызов
    for (const Part* p : usedParts) sum += p->getPrice();
    return sum;
}

int Order::deadlineDays() const {
    int days = 0;
    for (const auto& s : services) days += s->calculateDuration();
    return days;
}

std::string Order::plannedDate() const { return dates::addWorkdays(acceptDate, deadlineDays()); }

int Order::daysLeft(const std::string& today) const {
    return dates::daysBetween(today, plannedDate());
}

void Order::resolveLinks(Workshop& w) {
    client = &w.clients.findById(links.clientId);
    watch  = &w.watches.findById(links.watchId);
    master = links.masterId != 0 ? &w.masters.findById(links.masterId) : nullptr;
    usedParts.clear();
    for (int partId : links.partIds) usedParts.push_back(&w.parts.findById(partId));
}

std::ostream& operator<<(std::ostream& os, const Order& o) {
    os << o.id << ' ' << (o.client ? o.client->getId() : o.links.clientId) << ' '
       << (o.watch ? o.watch->getId() : o.links.watchId) << ' '
       << (o.master ? o.master->getId() : 0) << ' ' << std::quoted(o.acceptDate) << ' '
       << std::quoted(o.issueDate) << ' ' << static_cast<int>(o.status) << ' ' << o.services.size() << ' ';
    for (const auto& s : o.services) s->write(os);
    os << o.usedParts.size();
    for (const Part* p : o.usedParts) os << ' ' << p->getId();
    return os << '\n';
}

std::istream& operator>>(std::istream& is, Order& o) {
    int st = 0;
    std::size_t n = 0;
    if (!(is >> o.id >> o.links.clientId >> o.links.watchId >> o.links.masterId
             >> std::quoted(o.acceptDate) >> std::quoted(o.issueDate) >> st >> n)) return is;
    if (st < 0 || st > static_cast<int>(OrderStatus::Issued)) {
        is.setstate(std::ios::failbit);
        return is;
    }
    if (n > 1000) {                       // защита от повреждённых данных
        is.setstate(std::ios::failbit);
        return is;
    }
    o.status = static_cast<OrderStatus>(st);
    o.services.clear();
    for (std::size_t i = 0; i < n; ++i) {
        auto s = Service::read(is);
        if (!s) {
            is.setstate(std::ios::failbit);
            return is;
        }
        o.services.push_back(std::move(s));
    }
    if (!(is >> n)) return is;
    if (n > 100000) {
        is.setstate(std::ios::failbit);
        return is;
    }
    o.links.partIds.assign(n, 0);
    for (int& id : o.links.partIds) is >> id;
    return is;
}
