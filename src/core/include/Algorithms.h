#pragma once
#include <cstddef>
#include <iterator>
#include <list>
#include <utility>
#include <vector>

namespace algo {

template <typename T, typename Cmp>
void mergeSortRange(std::vector<T>& v, std::size_t lo, std::size_t hi, Cmp& cmp) {
    if (hi - lo < 2) return;
    std::size_t mid = lo + (hi - lo) / 2;
    mergeSortRange(v, lo, mid, cmp);
    mergeSortRange(v, mid, hi, cmp);

    std::vector<T> tmp;
    tmp.reserve(hi - lo);
    std::size_t i = lo, j = mid;
    while (i < mid && j < hi) {
        if (cmp(v[j], v[i])) tmp.push_back(std::move(v[j++]));
        else                 tmp.push_back(std::move(v[i++]));
    }
    while (i < mid) tmp.push_back(std::move(v[i++]));
    while (j < hi)  tmp.push_back(std::move(v[j++]));
    std::move(tmp.begin(), tmp.end(), v.begin() + static_cast<std::ptrdiff_t>(lo));
}

template <typename T, typename Cmp>
void mergeSort(std::vector<T>& v, Cmp cmp) {
    mergeSortRange(v, 0, v.size(), cmp);
}

template <typename T, typename Cmp>
void mergeSort(std::list<T>& lst, Cmp cmp) {
    if (lst.size() < 2) return;
    std::list<T> right;
    auto mid = std::next(lst.begin(), static_cast<std::ptrdiff_t>(lst.size() / 2));
    right.splice(right.begin(), lst, mid, lst.end());   

    mergeSort(lst, cmp);
    mergeSort(right, cmp);

    auto it = lst.begin();
    while (it != lst.end() && !right.empty()) {
        if (cmp(right.front(), *it)) lst.splice(it, right, right.begin());
        else ++it;
    }
    lst.splice(lst.end(), right);                     
}

}
