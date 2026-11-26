#pragma once
// Каталог услуг (прайс-лист). Набор услуг зависит от типа механизма часов, цена работы
// зависит от класса марки. Элементы питания и другие детали добавляются в заказ отдельно.
#include <memory>
#include <string>
#include <vector>

#include "Service.h"

// Какой дополнительный параметр нужен услуге.
enum class ServiceExtra { None, Disassembly, Complications };

struct ServiceTemplate {
    std::string name;
    std::string mechanism;     // тип механизма часов (см. Watch::mechanismTypes)
    double basePrice;          // цена работы для марки обычного класса, BYN
    ServiceExtra extra;
};

struct BrandInfo {
    std::string title;         // «обычная», «средняя», «премиум»
    double factor;             // множитель цены работы
};

class ServiceCatalog {
public:
    static const std::vector<ServiceTemplate>& all();
    // Услуги, подходящие часам с указанным типом механизма.
    static std::vector<const ServiceTemplate*> forMechanism(const std::string& mechanismType);
    static BrandInfo brandInfo(const std::string& brand);

    // Создаёт услугу из шаблона. extraValue: уровень разборки 1..3 (Disassembly)
    // или число функций 1..10 (Complications).
    static std::shared_ptr<Service> create(const ServiceTemplate& t, int complexity, int extraValue,
                                           double brandFactor);
};
