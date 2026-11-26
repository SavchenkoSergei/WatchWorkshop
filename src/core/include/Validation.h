#pragma once
// Проверка вводимых данных. Используется и ядром (Workshop), и диалогами интерфейса.
#include <cctype>
#include <string>

namespace validation {

inline std::string trim(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

// Фамилия, имя: буквы (латиница и кириллица в UTF-8), пробел, дефис, апостроф; без цифр.
inline bool isValidName(const std::string& s) {
    if (s.empty() || s.size() > 80) return false;
    bool hasLetter = false;
    for (unsigned char c : s) {
        if (c >= 0x80 || std::isalpha(c)) { hasLetter = true; continue; }   // байты UTF-8 - буквы
        if (c == ' ' || c == '-' || c == '\'') continue;
        return false;
    }
    return hasLetter;
}

// Телефон: цифры, пробелы, скобки, дефисы, «+» только в начале; от 7 до 15 цифр.
inline bool isValidPhone(const std::string& s) {
    int digits = 0;
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (std::isdigit(c)) ++digits;
        else if (c == '+' && i == 0) {}
        else if (c == ' ' || c == '-' || c == '(' || c == ')') {}
        else return false;
    }
    return digits >= 7 && digits <= 15;
}

// Приводит телефон к виду «+375 29 824 9433» (код страны, код оператора, 3 цифры, 4 цифры).
// Распознаются белорусские номера (+375..., 80..., 9 цифр без кода) и российские (+7 и 10 цифр);
// остальные номера возвращаются без изменений.
inline std::string formatPhone(const std::string& s) {
    std::string d;
    for (unsigned char c : s)
        if (std::isdigit(c)) d += static_cast<char>(c);
    auto belarus = [](const std::string& n) {                 // n - 9 цифр: оператор + номер
        return "+375 " + n.substr(0, 2) + " " + n.substr(2, 3) + " " + n.substr(5, 4);
    };
    if (d.size() == 12 && d.compare(0, 3, "375") == 0) return belarus(d.substr(3));
    if (d.size() == 11 && d.compare(0, 2, "80") == 0) return belarus(d.substr(2));
    if (d.size() == 9) return belarus(d);
    if (d.size() == 11 && d[0] == '7')
        return "+7 " + d.substr(1, 3) + " " + d.substr(4, 3) + " " + d.substr(7, 4);
    return trim(s);
}

// Email необязателен: пустая строка допустима, иначе нужен вид имя@домен.зона.
inline bool isValidEmail(const std::string& s) {
    if (s.empty()) return true;
    std::size_t at = s.find('@');
    if (at == std::string::npos || at == 0 || s.find('@', at + 1) != std::string::npos) return false;
    for (unsigned char c : s)
        if (std::isspace(c)) return false;
    std::size_t dot = s.find('.', at + 1);
    return dot != std::string::npos && dot > at + 1 && dot + 1 < s.size();
}

// Серийный номер: латинские буквы, цифры и знаки - _ / .
inline bool isValidSerial(const std::string& s) {
    if (s.empty() || s.size() > 40) return false;
    bool hasAlnum = false;
    for (unsigned char c : s) {
        if (c < 0x80 && std::isalnum(c)) { hasAlnum = true; continue; }
        if (c == '-' || c == '_' || c == '/' || c == '.') continue;
        return false;
    }
    return hasAlnum;
}

// Произвольный текст (марка, модель, название, категория): непустой, не длиннее 80 символов.
inline bool isValidText(const std::string& s) {
    std::string t = trim(s);
    return !t.empty() && s.size() <= 80;
}

// Логин: от 3 до 30 символов, латинские буквы, цифры, «_».
inline bool isValidLogin(const std::string& s) {
    if (s.size() < 3 || s.size() > 30) return false;
    for (unsigned char c : s)
        if (!(c < 0x80 && (std::isalnum(c) || c == '_'))) return false;
    return true;
}

// Приводит строку к виду для сравнения: без пробелов по краям, строчные буквы
// (латиница и кириллица в UTF-8), одиночные пробелы. Нужна, чтобы «Элемент питания»
// и «элемент  питания » считались одной деталью.
inline std::string foldCase(const std::string& s) {
    std::string t = trim(s), out;
    bool space = false;
    for (std::size_t i = 0; i < t.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(t[i]);
        if (std::isspace(c)) {
            if (!space) out += ' ';
            space = true;
            continue;
        }
        space = false;
        if (c == 0xD0 && i + 1 < t.size()) {                      // кириллица: заглавные -> строчные
            unsigned char n = static_cast<unsigned char>(t[i + 1]);
            if (n >= 0x90 && n <= 0x9F) { out += static_cast<char>(0xD0); out += static_cast<char>(n + 0x20); ++i; continue; }
            if (n >= 0xA0 && n <= 0xAF) { out += static_cast<char>(0xD1); out += static_cast<char>(n - 0x20); ++i; continue; }
            if (n == 0x81)              { out += static_cast<char>(0xD1); out += static_cast<char>(0x91); ++i; continue; }
        }
        out += static_cast<char>(c < 0x80 ? std::tolower(c) : c);
    }
    return out;
}

} // namespace validation
