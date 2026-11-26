#pragma once

#include <chrono>
#include <cstdio>
#include <ctime>
#include <optional>
#include <string>

namespace dates {

using Day = std::chrono::sys_days;

inline std::string format(Day d) {
    std::chrono::year_month_day ymd{d};
    char buf[16];
    std::snprintf(buf, sizeof buf, "%04d-%02u-%02u", static_cast<int>(ymd.year()),
                  static_cast<unsigned>(ymd.month()), static_cast<unsigned>(ymd.day()));
    return buf;
}

inline std::optional<Day> parse(const std::string& s) {
    int y = 0;
    unsigned m = 0, d = 0;
    if (s.size() != 10 || std::sscanf(s.c_str(), "%4d-%2u-%2u", &y, &m, &d) != 3) return std::nullopt;
    std::chrono::year_month_day ymd{std::chrono::year{y}, std::chrono::month{m}, std::chrono::day{d}};
    if (!ymd.ok()) return std::nullopt;
    return Day{ymd};
}

inline std::string today() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::chrono::year_month_day ymd{std::chrono::year{tm.tm_year + 1900},
                                    std::chrono::month{static_cast<unsigned>(tm.tm_mon + 1)},
                                    std::chrono::day{static_cast<unsigned>(tm.tm_mday)}};
    return format(Day{ymd});
}

inline std::string now() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[24];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &tm);
    return buf;
}
inline std::string addWorkdays(const std::string& date, int n) {
    auto start = parse(date);
    if (!start) return date;
    Day cur = *start;
    while (n > 0) {
        cur += std::chrono::days{1};
        std::chrono::weekday wd{cur};
        if (wd != std::chrono::Saturday && wd != std::chrono::Sunday) --n;
    }
    return format(cur);
}

inline int daysBetween(const std::string& from, const std::string& to) {
    auto a = parse(from), b = parse(to);
    if (!a || !b) return 0;
    return static_cast<int>((*b - *a).count());
}

}
