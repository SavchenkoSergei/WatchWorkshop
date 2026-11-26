#include "Workshop.h"

#include <algorithm>

#include "Dates.h"
#include "Exceptions.h"
#include "Validation.h"

namespace {

void checkName(const std::string& v, const char* field) {
    if (!validation::isValidName(v)) throw InvalidInputException(field);
}
void checkText(const std::string& v, const char* field) {
    if (!validation::isValidText(v)) throw InvalidInputException(field);
}

void checkClient(const Client& c) {
    checkName(c.getLastName(), "Фамилия");
    checkName(c.getFirstName(), "Имя");
    if (!validation::isValidPhone(c.getPhone())) throw InvalidInputException("Телефон");
    if (!validation::isValidEmail(c.getEmail())) throw InvalidInputException("Email");
}

void checkMaster(const Master& m) {
    checkName(m.getLastName(), "Фамилия");
    checkName(m.getFirstName(), "Имя");
    if (!m.getPhone().empty() && !validation::isValidPhone(m.getPhone()))
        throw InvalidInputException("Телефон");
}

void checkPart(const Part& p) {
    checkText(p.getName(), "Наименование");
    if (p.getQuantity() < 0) throw InvalidInputException("Количество");
    if (p.getPrice() < 0) throw InvalidInputException("Цена");
}

void checkWatchFields(const Watch& w) {
    checkText(w.getBrand(), "Марка");
    checkText(w.getModel(), "Модель");
    if (!validation::isValidSerial(w.getSerialNumber())) throw InvalidInputException("Серийный номер");
    const auto& types = Watch::mechanismTypes();
    if (std::find(types.begin(), types.end(), w.getMechanismType()) == types.end())
        throw InvalidInputException("Тип механизма");
}

void requireEditable(const Order& order) {
    if (order.getStatus() >= OrderStatus::Ready)
        throw InvalidInputException("Заказ готов или выдан: изменять его нельзя");
}

std::string personName(const std::string& last, const std::string& first) { return last + " " + first; }
std::string orderLabel(const Order& o) { return "заказ №" + std::to_string(o.getId()); }

} 

Workshop::Workshop(std::string dataDir) : storage(std::move(dataDir)) {}



void Workshop::setActor(const std::string& login, Role role, int masterId) {
    actor = Actor{login, role, masterId};
    if (role != Role::Guest) record("Вход в систему", "роль: " + roleToString(role));
}

void Workshop::require(std::initializer_list<Role> roles, const std::string& action) const {
    if (std::find(roles.begin(), roles.end(), actor.role) == roles.end())
        throw AccessDeniedException(action);
}

void Workshop::requireOwnerOr(const Order& order, std::initializer_list<Role> roles,
                              const std::string& action) const {
    if (actor.role == Role::Master) {
        if (!order.getMaster() || order.getMaster()->getId() != actor.masterId)
            throw AccessDeniedException(action + ": это может сделать только мастер, взявший заказ");
        return;
    }
    require(roles, action);
}

void Workshop::record(const std::string& action, const std::string& details) {
    actionLog.add(LogEntry(actionLog.nextId(), dates::now(), actor.login, actor.role, action, details));
}



void Workshop::addClient(const Client& client) {
    require({Role::Reception, Role::Admin}, "клиентов добавляет ресепшен");
    checkClient(client);
    Client c(clients.nextId(), client.getLastName(), client.getFirstName(),
             validation::formatPhone(client.getPhone()), client.getEmail());
    int id = c.getId();
    clients.add(std::move(c));
    record("Клиент добавлен", personName(client.getLastName(), client.getFirstName()) + " (id " + std::to_string(id) + ")");
}

void Workshop::addMaster(const Master& master) {
    require({Role::Admin}, "мастеров добавляет администратор");
    checkMaster(master);
    Master m(masters.nextId(), master.getLastName(), master.getFirstName(), master.getSpecialization(),
             validation::formatPhone(master.getPhone()));
    int id = m.getId();
    masters.add(std::move(m));
    record("Мастер добавлен", personName(master.getLastName(), master.getFirstName()) + " (id " + std::to_string(id) + ")");
}

void Workshop::acceptWatch(const Watch& watch) {
    require({Role::Reception, Role::Admin}, "часы принимает ресепшен");
    checkWatchFields(watch);
    Client& owner = clients.findById(watch.getClientId());      
    const std::string& serial = watch.getSerialNumber();
    if (watches.findIf([&serial](const Watch& w) { return w.getSerialNumber() == serial; }) != watches.end())
        throw InvalidInputException("Серийный номер (такие часы уже приняты)");
    Watch w = watch;
    w.setId(watches.nextId());
    w.setClient(&owner);
    int id = w.getId();
    watches.add(std::move(w));
    record("Часы приняты", watch.getBrand() + " " + watch.getModel() + " (" + serial + "), владелец: " +
                               personName(owner.getLastName(), owner.getFirstName()) + ", id " + std::to_string(id));
}

void Workshop::updateClient(int id, const Client& client) {
    require({Role::Reception, Role::Admin}, "клиентов изменяет ресепшен");
    checkClient(client);
    Client c(id, client.getLastName(), client.getFirstName(),
             validation::formatPhone(client.getPhone()), client.getEmail());
    clients.update(id, c);
    record("Клиент изменён", personName(client.getLastName(), client.getFirstName()) + " (id " + std::to_string(id) + ")");
}

void Workshop::updateWatch(int id, const Watch& watch) {
    require({Role::Reception, Role::Admin}, "часы изменяет ресепшен");
    checkWatchFields(watch);
    Client& owner = clients.findById(watch.getClientId());
    const std::string& serial = watch.getSerialNumber();
    if (watches.findIf([id, &serial](const Watch& w) { return w.getId() != id && w.getSerialNumber() == serial; })
        != watches.end())
        throw InvalidInputException("Серийный номер (такие часы уже приняты)");
    Watch w = watch;
    w.setId(id);
    w.setClient(&owner);
    watches.update(id, w);
    record("Часы изменены", watch.getBrand() + " " + watch.getModel() + " (" + serial + "), id " + std::to_string(id));
}

void Workshop::updateMaster(int id, const Master& master) {
    require({Role::Admin}, "мастеров изменяет администратор");
    checkMaster(master);
    Master m(id, master.getLastName(), master.getFirstName(), master.getSpecialization(),
             validation::formatPhone(master.getPhone()));
    masters.update(id, m);
    record("Мастер изменён", personName(master.getLastName(), master.getFirstName()) + " (id " + std::to_string(id) + ")");
}



bool Workshop::addPart(const Part& part) {
    require({Role::Reception, Role::Admin}, "детали на склад принимает ресепшен");
    checkPart(part);
    std::string name = validation::foldCase(part.getName());
    std::string category = validation::foldCase(part.getCategory());
    auto same = parts.findIf([&name, &category](const Part& p) {
        return validation::foldCase(p.getName()) == name && validation::foldCase(p.getCategory()) == category;
        });
    if (same != parts.end()) {                                
        if (part.getQuantity() <= 0) throw InvalidInputException("Количество (укажите, сколько привезли)");
        same->addStock(part.getQuantity());
        record("Деталь: приём партии (уже была на складе)", same->getName() + ", количество +" +
                   std::to_string(part.getQuantity()) + ", остаток " + std::to_string(same->getQuantity()));
        return true;
    }
    Part p = part;
    p.setId(parts.nextId());
    parts.add(std::move(p));
    record("Деталь добавлена", part.getName() + ", количество " + std::to_string(part.getQuantity()));
    return false;
}

void Workshop::updatePart(int id, const Part& part) {
    require({Role::Admin}, "детали изменяет администратор");
    checkPart(part);
    Part p = part;
    p.setId(id);
    parts.update(id, p);
    record("Деталь изменена", part.getName() + " (id " + std::to_string(id) + "), остаток " +
                                  std::to_string(part.getQuantity()));
}

void Workshop::restockPart(int partId, int qty) {
    require({Role::Reception, Role::Admin}, "партии принимает ресепшен");
    Part& part = parts.findById(partId);
    part.addStock(qty);
    record("Деталь: приём партии", part.getName() + ", количество +" + std::to_string(qty) +
                                       ", остаток " + std::to_string(part.getQuantity()));
}

void Workshop::writeOffPart(int partId, int qty) {
    require({Role::Master, Role::Admin}, "детали списывает мастер");
    Part& part = parts.findById(partId);
    part.writeOff(qty);
    record("Деталь списана", part.getName() + ", количество " + std::to_string(qty) +
                                 ", остаток " + std::to_string(part.getQuantity()));
}



void Workshop::removeClient(int id) {
    require({Role::Admin}, "удаляет администратор");
    Client& client = clients.findById(id);
    std::string who = personName(client.getLastName(), client.getFirstName());
    if (watches.findIf([id](const Watch& w) { return w.getClientId() == id; }) != watches.end())
        throw ObjectInUseException("у клиента есть принятые часы");
    if (orders.findIf([id](const Order& o) { return o.getClient() && o.getClient()->getId() == id; })
        != orders.end())
        throw ObjectInUseException("у клиента есть заказы");
    clients.removeById(id);
    record("Клиент удалён", who + " (id " + std::to_string(id) + ")");
}

void Workshop::removeWatch(int id) {
    require({Role::Admin}, "удаляет администратор");
    Watch& watch = watches.findById(id);
    std::string what = watch.getBrand() + " " + watch.getModel() + " (" + watch.getSerialNumber() + ")";
    if (orders.findIf([id](const Order& o) { return o.getWatch() && o.getWatch()->getId() == id; })
        != orders.end())
        throw ObjectInUseException("эти часы указаны в заказе");
    watches.removeById(id);
    record("Часы удалены", what);
}

void Workshop::removeMaster(int id) {
    require({Role::Admin}, "удаляет администратор");
    Master& master = masters.findById(id);
    std::string who = personName(master.getLastName(), master.getFirstName());
    if (orders.findIf([id](const Order& o) { return o.getMaster() && o.getMaster()->getId() == id; })
        != orders.end())
        throw ObjectInUseException("мастер назначен на заказ");
    if (accounts.findIf([id](const Account& a) { return a.getMasterId() == id; }) != accounts.end())
        throw ObjectInUseException("у мастера есть учётная запись");
    masters.removeById(id);
    record("Мастер удалён", who + " (id " + std::to_string(id) + ")");
}

void Workshop::removePart(int id) {
    require({Role::Admin}, "удаляет администратор");
    Part& part = parts.findById(id);
    Part* ptr = &part;
    std::string name = part.getName();
    if (orders.findIf([ptr](const Order& o) {
            const auto& used = o.getUsedParts();
            return std::find(used.begin(), used.end(), ptr) != used.end();
        }) != orders.end())
        throw ObjectInUseException("деталь используется в заказе");
    int left = part.getQuantity();
    parts.removeById(id);
    record("Деталь удалена", name + ", остаток на момент удаления " + std::to_string(left));
}


void Workshop::removeOrder(int id) {
    require({Role::Admin}, "удаляет администратор");
    Order& order = orders.findById(id);
    std::string label = orderLabel(order);
    int returned = 0;
    if (order.getStatus() < OrderStatus::Ready)
        for (Part* p : order.getUsedParts()) {
            p->addStock(1);
            ++returned;
        }
    orders.removeById(id);
    record("Заказ удалён", label + (returned ? ", возвращено деталей на склад: " + std::to_string(returned) : ""));
}



int Workshop::activeOrderId(int watchId) const {
    auto it = orders.findIf([watchId](const Order& o) {
        return o.getWatch() && o.getWatch()->getId() == watchId && o.getStatus() != OrderStatus::Issued;
    });
    return it == orders.end() ? 0 : it->getId();
}

int Workshop::createOrder(int clientId, int watchId, int masterId, std::shared_ptr<Service> service) {
    require({Role::Reception, Role::Admin}, "заказы оформляет ресепшен");
    Client& client = clients.findById(clientId);
    Watch& watch = watches.findById(watchId);
    if (watch.getClientId() != clientId) throw InvalidInputException("Часы (принадлежат другому клиенту)");
    if (int active = activeOrderId(watchId))
        throw InvalidInputException("На эти часы уже оформлен заказ №" + std::to_string(active) +
                                    " (он ещё не выдан)");
    if (!service) throw InvalidInputException("Услуга");
    Master* master = masterId != 0 ? &masters.findById(masterId) : nullptr;
    int id = orders.nextId();
    std::string serviceName = service->getName();
    orders.add(Order(id, &client, &watch, master, std::move(service), dates::today()));
    record("Заказ создан", "заказ №" + std::to_string(id) + ": " + watch.getBrand() + " " + watch.getModel() +
                               ", клиент " + personName(client.getLastName(), client.getFirstName()) +
                               ", услуга: " + serviceName);
    return id;
}

void Workshop::assignMaster(int orderId, int masterId) {
    require({Role::Reception, Role::Admin}, "мастера назначает ресепшен");
    Order& order = orders.findById(orderId);
    requireEditable(order);
    Master& master = masters.findById(masterId);
    order.assignMaster(&master);
    record("Мастер назначен", orderLabel(order) + ": " + personName(master.getLastName(), master.getFirstName()));
}


void Workshop::takeOrder(int orderId) {
    require({Role::Master}, "брать заказ может только мастер");
    if (actor.masterId == 0) throw InvalidInputException("к вашей учётной записи не привязан мастер");
    Order& order = orders.findById(orderId);
    Master& master = masters.findById(actor.masterId);
    requireEditable(order);
    if (order.getMaster()) throw InvalidInputException("Заказ уже взят мастером");
    if (order.isComplex())
        throw AccessDeniedException("сложный заказ (сложность 4–5) назначает ресепшен");
    order.assignMaster(&master);
    record("Заказ взят мастером", orderLabel(order) + ": " + personName(master.getLastName(), master.getFirstName()));
}

void Workshop::addService(int orderId, std::shared_ptr<Service> service) {
    Order& order = orders.findById(orderId);
    requireOwnerOr(order, {Role::Reception, Role::Admin}, "услуги добавляет мастер своего заказа или ресепшен");
    requireEditable(order);
    std::string name = service ? service->getName() : "";
    order.addService(std::move(service));
    record("Услуга добавлена в заказ", orderLabel(order) + ": " + name);
}

void Workshop::removeService(int orderId, std::size_t serviceIndex) {
    require({Role::Reception, Role::Admin}, "услуги удаляет ресепшен");
    Order& order = orders.findById(orderId);
    requireEditable(order);
    std::string name = serviceIndex < order.getServices().size() ? order.getServices()[serviceIndex]->getName() : "";
    order.removeService(serviceIndex);
    record("Услуга удалена из заказа", orderLabel(order) + ": " + name);
}


void Workshop::addUsedPart(int orderId, int partId) {
    Order& order = orders.findById(orderId);
    Part& part = parts.findById(partId);
    requireOwnerOr(order, {Role::Admin}, "детали в заказ добавляет мастер своего заказа");
    requireEditable(order);
    part.writeOff(1);
    order.addUsedPart(&part);
    record("Деталь добавлена в заказ", orderLabel(order) + ": " + part.getName() + ", количество 1, остаток " +
                                           std::to_string(part.getQuantity()));
}


void Workshop::removeUsedPart(int orderId, int partId) {
    Order& order = orders.findById(orderId);
    Part& part = parts.findById(partId);
    requireOwnerOr(order, {Role::Admin}, "детали из заказа убирает мастер своего заказа");
    requireEditable(order);
    order.removeUsedPart(&part);      
    part.addStock(1);
    record("Деталь убрана из заказа", orderLabel(order) + ": " + part.getName() + ", количество 1, остаток " +
                                          std::to_string(part.getQuantity()));
}

bool Workshop::isStatusAllowed(Role who, OrderStatus current, OrderStatus next) {
    if (who == Role::Admin) return true;
    if (who == Role::Guest) return false;
    if (current == OrderStatus::Issued) return false;          
    if (who == Role::Master)
        return next == OrderStatus::Diagnostics || next == OrderStatus::InProgress ||
               next == OrderStatus::Ready;
    return next == OrderStatus::Issued && current == OrderStatus::Ready;   
}


void Workshop::changeStatus(int orderId, OrderStatus next) {
    Order& order = orders.findById(orderId);
    OrderStatus prev = order.getStatus();
    if (next == prev) return;

    if (actor.role == Role::Master &&
        (!order.getMaster() || order.getMaster()->getId() != actor.masterId))
        throw AccessDeniedException("статус заказа меняет мастер, который его взял");
    if (!isStatusAllowed(actor.role, prev, next)) {
        if (prev == OrderStatus::Issued)
            throw AccessDeniedException("выданный заказ может изменить только администратор");
        if (next == OrderStatus::Issued && actor.role == Role::Reception)
            throw InvalidInputException("Выдать можно только готовый заказ");
        if (next == OrderStatus::Issued)
            throw AccessDeniedException("выдачу заказа клиенту оформляет ресепшен");
        throw AccessDeniedException("эта роль не может установить статус «" + statusToString(next) + "»");
    }
    if (next == OrderStatus::Issued && prev != OrderStatus::Ready && actor.role != Role::Admin)
        throw InvalidInputException("Выдать можно только готовый заказ");
    if (next >= OrderStatus::InProgress && !order.getMaster())
        throw InvalidInputException("Мастер не назначен (нужен для статусов «В работе», «Готов», «Выдан»)");

    order.setStatus(next);
    if (next == OrderStatus::Issued) order.setIssueDate(dates::today());   
    else if (prev == OrderStatus::Issued) order.setIssueDate("");         
    record("Смена статуса", orderLabel(order) + ": " + statusToString(prev) + " → " + statusToString(next));
}



Account Workshop::login(const std::string& login, const std::string& password) const {
    auto it = accounts.findIf([&login](const Account& a) { return a.getLogin() == login; });
    if (it == accounts.end() || !it->checkPassword(password))
        throw InvalidInputException("неверный логин или пароль");
    return *it;
}

void Workshop::addAccount(const std::string& login, const std::string& password, Role role, int masterId) {
    require({Role::Admin}, "учётные записи создаёт администратор");
    if (!validation::isValidLogin(login)) throw InvalidInputException("Логин (3-30 латинских букв, цифр или _)");
    if (password.size() < 4) throw InvalidInputException("Пароль (не короче 4 символов)");
    if (role == Role::Guest) throw InvalidInputException("Роль");
    if (accounts.findIf([&login](const Account& a) { return a.getLogin() == login; }) != accounts.end())
        throw InvalidInputException("Логин (уже занят)");
    if (role == Role::Master) {
        masters.findById(masterId);                           
        if (accounts.findIf([masterId](const Account& a) { return a.getMasterId() == masterId; })
            != accounts.end())
            throw InvalidInputException("Мастер (у него уже есть учётная запись)");
    } else {
        masterId = 0;
    }
    accounts.add(Account(accounts.nextId(), login, Account::hashPassword(login, password), role, masterId));
    record("Учётная запись создана", login + ", роль: " + roleToString(role));
}

void Workshop::removeAccount(int id) {
    require({Role::Admin}, "учётные записи удаляет администратор");
    const Account& acc = accounts.findById(id);
    std::string login = acc.getLogin();
    if (acc.getRole() == Role::Admin) {
        auto admins = std::count_if(accounts.begin(), accounts.end(),
                                    [](const Account& a) { return a.getRole() == Role::Admin; });
        if (admins <= 1) throw ObjectInUseException("это последний администратор");
    }
    accounts.removeById(id);
    record("Учётная запись удалена", login);
}

void Workshop::changePassword(int accountId, const std::string& newPassword) {
    Account& acc = accounts.findById(accountId);
    if (actor.role != Role::Admin && actor.login != acc.getLogin())
        throw AccessDeniedException("пароль можно сменить только свой");
    if (newPassword.size() < 4) throw InvalidInputException("Пароль (не короче 4 символов)");
    acc.setPasswordHash(Account::hashPassword(acc.getLogin(), newPassword));
    record("Пароль изменён", acc.getLogin());
}

void Workshop::ensureDefaultAccount() {
    if (accounts.size() == 0)
        accounts.add(Account(1, "admin", Account::hashPassword("admin", "admin"), Role::Admin, 0));
}



void Workshop::saveAll() { storage.saveAll(*this); }
void Workshop::loadAll() { storage.loadAll(*this); }