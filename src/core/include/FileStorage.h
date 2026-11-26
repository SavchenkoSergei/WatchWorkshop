#pragma once
#include <string>

#include "Repository.h"

class Workshop;

class FileStorage {
public:
    explicit FileStorage(std::string dataDir = "data");

    const std::string& getDataDir() const { return dataDir; }

    void saveAll(const Workshop& w) const;

    void loadAll(Workshop& w) const;

private:
    template <typename T, typename C>
    void save(const Repository<T, C>& repo, const std::string& file) const;

    template <typename T, typename C>
    void load(Repository<T, C>& repo, const std::string& file) const;

    std::string path(const std::string& file) const;
    void quarantine() const;

    std::string dataDir;
    std::string clientsFile = "clients.txt";
    std::string watchesFile = "watches.txt";
    std::string mastersFile = "masters.txt";
    std::string ordersFile  = "orders.txt";
    std::string partsFile   = "parts.txt";
    std::string accountsFile = "accounts.txt";
    std::string logFile = "log.txt";
};
