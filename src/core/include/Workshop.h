#pragma once
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
    struct Actor {
        std::string login = "система";
        Role role = Role::Admin;
        int masterId = 0;        
    };

    explicit Workshop(std::string dataDir = "data");

    void setActor(const std::string& login, Role role, int masterId);
    const Actor& getActor() const { return actor; }

    void addClient(const Client& client);
    void addMaster(const Master& master);        
    void acceptWatch(const Watch& watch);
    void updateClient(int id, const Client& client);
    void updateWatch(int id, const Watch& watch);
    void updateMaster(int id, const Master& master);     

    bool addPart(const Part& part);
    void updatePart(int id, const Part& part);            
    void restockPart(int partId, int qty);                
    void writeOffPart(int partId, int qty);             

    void removeClient(int id);
    void removeWatch(int id);
    void removeMaster(int id);
    void removePart(int id);
    void removeOrder(int id);     

    int createOrder(int clientId, int watchId, int masterId, std::shared_ptr<Service> service);
    int activeOrderId(int watchId) const;                
    void assignMaster(int orderId, int masterId);          
    void takeOrder(int orderId);
    void addService(int orderId, std::shared_ptr<Service> service);
    void removeService(int orderId, std::size_t serviceIndex);
    void addUsedPart(int orderId, int partId);   
    void removeUsedPart(int orderId, int partId); 

    void changeStatus(int orderId, OrderStatus newStatus);
    static bool isStatusAllowed(Role actor, OrderStatus current, OrderStatus next);

    Account login(const std::string& login, const std::string& password) const;
    void addAccount(const std::string& login, const std::string& password, Role role, int masterId);
    void removeAccount(int id);
    void changePassword(int accountId, const std::string& newPassword);  

    void ensureDefaultAccount();


    const std::string& getDataDir() const { return storage.getDataDir(); }
    void saveAll();
    void loadAll();

private:
    void require(std::initializer_list<Role> roles, const std::string& action) const;

    void requireOwnerOr(const Order& order, std::initializer_list<Role> roles, const std::string& action) const;
    void record(const std::string& action, const std::string& details);   

    ClientRepo clients;
    WatchRepo watches;
    MasterRepo masters;
    OrderRepo orders;
    PartRepo parts;
    AccountRepo accounts;
    LogRepo actionLog;
    FileStorage storage;
    Actor actor;

    friend class FileStorage;
    friend class Order;
    friend class Watch;
    friend class MainWindow;
    friend class WorkshopTests;
};
