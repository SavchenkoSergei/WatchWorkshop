#pragma once
#include <string>

// Статусы заказа. Порядок значений соответствует жизненному циклу ремонта.
enum class OrderStatus { Accepted = 0, Diagnostics, InProgress, Ready, Issued };

inline std::string statusToString(OrderStatus s) {
    switch (s) {
        case OrderStatus::Accepted:    return "Принят";
        case OrderStatus::Diagnostics: return "На диагностике";
        case OrderStatus::InProgress:  return "В работе";
        case OrderStatus::Ready:       return "Готов";
        case OrderStatus::Issued:      return "Выдан";
    }
    return "?";
}
