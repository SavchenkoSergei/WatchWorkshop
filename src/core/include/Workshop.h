#pragma once
// Фасад бизнес-логики мастерской. Интерфейс обращается к ядру только через Workshop.
// Права проверяются здесь, в ядре, по роли текущего пользователя (см. setActor);
// каждое изменение данных записывается в журнал действий.
#include <initializer_list>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "Account.h"
#include "Client.h"
#include "FileStorage.h"
#include "LogEntry.h"
#include "Master.h"
#include "Order.h"
#include "Part.h"
#include "Repository.h"
#include "Watch.h"

using ClientRepo  = Repository<Client, std::list<Client>>;
using WatchRepo   = Repository<Watch, std::list<Watch>>;
using MasterRepo  = Repository<Master, std::list<Master>>;
using PartRepo    = Repository<Part, std::list<Part>>;
using OrderRepo   = Repository<Order, std::vector<Order>>;
using AccountRepo = Repository<Account, std::list<Account>>;
using LogRepo     = Repository<LogEntry, std::vector<LogEntry>>;

class Workshop {
public:
    // Пользователь, от имени которого выполняются действия. По умолчанию - служебный
    // администратор «система» (запуск программы, проверки ядра).
    struct Actor {
        std::string login = "система";
        Role role = Role::Admin;
        int masterId = 0;          // мастер, связанный с учётной записью (для роли «Мастер»)
    };

    explicit Workshop(std::string dataDir = "data");

    // ---- текущий пользователь ----
    void setActor(const std::string& login, Role role, int masterId);
    const Actor& getActor() const { return actor; }

    // ---- клиенты, часы, мастера (клиенты и часы - ресепшен и администратор) ----
    void addClient(const Client& client);
    void addMaster(const Master& master);                  // администратор
    void acceptWatch(const Watch& watch);
    void updateClient(int id, const Client& client);
    void updateWatch(int id, const Watch& watch);
    void updateMaster(int id, const Master& master);       // администратор

    // ---- детали ----
    // Ресепшен и администратор принимают детали. Если такая деталь (название и категория)
    // уже есть на складе, её количество пополняется, и метод возвращает true.
    bool addPart(const Part& part);
    void updatePart(int id, const Part& part);             // администратор
    void restockPart(int partId, int qty);                 // ресепшен и администратор
    void writeOffPart(int partId, int qty);                // мастер и администратор

    // ---- удаление (администратор; запрещено, пока на объект ссылаются другие объекты) ----
    void removeClient(int id);
    void removeWatch(int id);
    void removeMaster(int id);
    void removePart(int id);
    void removeOrder(int id);     // детали незавершённого заказа возвращаются на склад

    // ---- заказы ----
    // Создаёт заказ (ресепшен, администратор); masterId == 0 - мастер не назначен.
    // На часы с незавершённым заказом второй заказ создать нельзя. Возвращает id заказа.
    int createOrder(int clientId, int watchId, int masterId, std::shared_ptr<Service> service);
    int activeOrderId(int watchId) const;                  // незавершённый заказ на часы или 0
    void assignMaster(int orderId, int masterId);          // ресепшен, администратор
    // Мастер берёт заказ сам: только если заказ ещё не взят, не готов и не сложный.
    void takeOrder(int orderId);
    // Услуги: добавляет мастер своего заказа, ресепшен, администратор; удаляют ресепшен и администратор.
    // Детали заказа: только мастер своего заказа и администратор.
    // У готового и выданного заказа услуги, детали и мастера менять нельзя.
    void addService(int orderId, std::shared_ptr<Service> service);
    void removeService(int orderId, std::size_t serviceIndex);
    void addUsedPart(int orderId, int partId);      // деталь сразу списывается со склада
    void removeUsedPart(int orderId, int partId);   // деталь возвращается на склад
    // Смена статуса: мастер (только своего заказа) ставит «На диагностике», «В работе», «Готов»;
    // ресепшен выдаёт готовый заказ («Выдан»); выданный заказ и «Принят» ставит только администратор.
    void changeStatus(int orderId, OrderStatus newStatus);
    static bool isStatusAllowed(Role actor, OrderStatus current, OrderStatus next);

    // ---- учётные записи сотрудников ----
    Account login(const std::string& login, const std::string& password) const;
    void addAccount(const std::string& login, const std::string& password, Role role, int masterId);
    void removeAccount(int id);
    void changePassword(int accountId, const std::string& newPassword);   // свой пароль или администратор
    // Если учётных записей нет (первый запуск), создаёт администратора admin / admin.
    void ensureDefaultAccount();

    // ---- файлы ----
    const std::string& getDataDir() const { return storage.getDataDir(); }
    void saveAll();
    void loadAll();

private:
    void require(std::initializer_list<Role> roles, const std::string& action) const;
    // Мастер действует только в своём заказе; остальным ролям нужно быть в списке roles.
    void requireOwnerOr(const Order& order, std::initializer_list<Role> roles, const std::string& action) const;
    void record(const std::string& action, const std::string& details);   // запись в журнал

    ClientRepo clients;
    WatchRepo watches;
    MasterRepo masters;
    OrderRepo orders;
    PartRepo parts;
    AccountRepo accounts;
    LogRepo actionLog;
    FileStorage storage;
    Actor actor;

    // Классы, которым нужен прямой доступ к репозиториям.
    friend class FileStorage;
    friend class Order;
    friend class Watch;
    friend class MainWindow;
    friend class WorkshopTests;
};
