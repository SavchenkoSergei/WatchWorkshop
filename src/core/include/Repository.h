#pragma once
// Шаблонный репозиторий: хранит коллекцию однотипных объектов в контейнере STL.
// Требования к T: метод getId(), копирование/присваивание.
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <list>
#include <utility>
#include <vector>

#include "Algorithms.h"
#include "Exceptions.h"

template <typename T, typename Container = std::vector<T>>
class Repository {
public:
    using iterator       = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    void add(const T& item) {
        ensureUnique(item.getId());
        items.push_back(item);
    }
    void add(T&& item) {
        ensureUnique(item.getId());
        items.push_back(std::move(item));
    }

    void removeById(int id) {
        auto it = findIf([id](const T& x) { return x.getId() == id; });
        if (it == items.end()) throw ObjectNotFoundException(id);
        items.erase(it);
    }

    T& findById(int id) {
        auto it = findIf([id](const T& x) { return x.getId() == id; });
        if (it == items.end()) throw ObjectNotFoundException(id);
        return *it;
    }
    const T& findById(int id) const {
        auto it = findIf([id](const T& x) { return x.getId() == id; });
        if (it == items.end()) throw ObjectNotFoundException(id);
        return *it;
    }

    template <typename Pred>
    iterator findIf(Pred pred) { return std::find_if(items.begin(), items.end(), pred); }
    template <typename Pred>
    const_iterator findIf(Pred pred) const { return std::find_if(items.begin(), items.end(), pred); }

    // Сортировка собственным алгоритмом (см. Algorithms.h).
    template <typename Cmp>
    void sortBy(Cmp cmp) { algo::mergeSort(items, cmp); }

    std::size_t size() const { return items.size(); }

    iterator begin() { return items.begin(); }
    iterator end() { return items.end(); }
    const_iterator begin() const { return items.begin(); }
    const_iterator end() const { return items.end(); }

    const Container& getAll() const { return items; }

    // Заменяет содержимое объекта на месте: адрес объекта не меняется,
    // поэтому указатели на него остаются действительными.
    void update(int id, const T& item) { findById(id) = item; }

    T& at(std::size_t index) {
        if (index >= items.size()) throw ObjectNotFoundException(static_cast<int>(index));
        return *std::next(items.begin(), static_cast<std::ptrdiff_t>(index));
    }
    const T& at(std::size_t index) const {
        if (index >= items.size()) throw ObjectNotFoundException(static_cast<int>(index));
        return *std::next(items.begin(), static_cast<std::ptrdiff_t>(index));
    }

    // Идентификатор для новой записи: максимальный существующий + 1.
    int nextId() const {
        int maxId = 0;
        for (const T& x : items) maxId = std::max(maxId, x.getId());
        return maxId + 1;
    }

    void clear() { items.clear(); }

private:
    void ensureUnique(int id) const {
        bool exists = std::any_of(items.begin(), items.end(),
                                  [id](const T& x) { return x.getId() == id; });
        if (exists) throw InvalidInputException("id");
    }

    Container items;
};
