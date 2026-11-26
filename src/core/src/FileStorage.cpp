#include "FileStorage.h"

#include <filesystem>
#include <fstream>

#include "Exceptions.h"
#include "Workshop.h"

namespace fs = std::filesystem;

FileStorage::FileStorage(std::string dataDir) : dataDir(std::move(dataDir)) {}

std::string FileStorage::path(const std::string& file) const {
    return (fs::path(dataDir) / file).string();
}

template <typename T, typename C>
void FileStorage::save(const Repository<T, C>& repo, const std::string& file) const {
    std::ofstream out(path(file));
    if (!out) throw FileCorruptedException(file);
    out << repo.size() << '\n';
    for (const T& item : repo) out << item;
    if (!out) throw FileCorruptedException(file);
}

template <typename T, typename C>
void FileStorage::load(Repository<T, C>& repo, const std::string& file) const {
    std::ifstream in(path(file));
    if (!in) return;                         
    std::size_t n = 0;
    if (!(in >> n)) throw FileCorruptedException(file);
    for (std::size_t i = 0; i < n; ++i) {
        T item;
        if (!(in >> item)) throw FileCorruptedException(file);
        repo.add(std::move(item));
    }
}

void FileStorage::quarantine() const {
    std::error_code ec;
    for (const std::string& file : {clientsFile, watchesFile, mastersFile, ordersFile, partsFile, accountsFile, logFile})
        if (fs::exists(path(file), ec)) fs::rename(path(file), path(file) + ".bak", ec);
}

void FileStorage::saveAll(const Workshop& w) const {
    std::error_code ec;
    fs::create_directories(dataDir, ec);
    save(w.clients, clientsFile);
    save(w.watches, watchesFile);
    save(w.masters, mastersFile);
    save(w.parts, partsFile);
    save(w.orders, ordersFile);
    save(w.accounts, accountsFile);
    save(w.actionLog, logFile);
}

void FileStorage::loadAll(Workshop& w) const {
    w.clients.clear();
    w.watches.clear();
    w.masters.clear();
    w.parts.clear();
    w.orders.clear();
    w.accounts.clear();
    w.actionLog.clear();

    std::string current;
    try {
        current = clientsFile;  load(w.clients, clientsFile);
        current = mastersFile;  load(w.masters, mastersFile);
        current = partsFile;    load(w.parts, partsFile);
        current = accountsFile; load(w.accounts, accountsFile);
        current = logFile;      load(w.actionLog, logFile);
        current = watchesFile;  load(w.watches, watchesFile);
        for (Watch& x : w.watches) x.resolveLinks(w);
        current = ordersFile;   load(w.orders, ordersFile);
        for (Order& o : w.orders) o.resolveLinks(w);
    } catch (const WorkshopException&) {
        quarantine();
        w.clients.clear();
        w.watches.clear();
        w.masters.clear();
        w.parts.clear();
        w.orders.clear();
        w.accounts.clear();
        w.actionLog.clear();
        throw FileCorruptedException(current);
    }
}
