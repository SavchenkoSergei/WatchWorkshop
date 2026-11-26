#include "ServiceCatalog.h"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "ChronographRepairService.h"
#include "Exceptions.h"
#include "MechanicalRepairService.h"
#include "QuartzRepairService.h"

const std::vector<ServiceTemplate>& ServiceCatalog::all() {
    static const std::vector<ServiceTemplate> list = {
  
        {"Диагностика", "кварцевый", 10, ServiceExtra::None},
        {"Замена элемента питания", "кварцевый", 8, ServiceExtra::None},
        {"Замена стекла", "кварцевый", 25, ServiceExtra::None},
        {"Замена ремешка или браслета", "кварцевый", 12, ServiceExtra::None},
        {"Проверка и восстановление водозащиты", "кварцевый", 30, ServiceExtra::None},
        {"Замена кварцевого механизма", "кварцевый", 45, ServiceExtra::None},

        {"Диагностика", "механический", 15, ServiceExtra::None},
        {"Чистка и смазка механизма", "механический", 60, ServiceExtra::Disassembly},
        {"Регулировка хода", "механический", 40, ServiceExtra::None},
        {"Замена заводной головки и штока", "механический", 35, ServiceExtra::None},
        {"Замена заводной пружины", "механический", 55, ServiceExtra::Disassembly},
        {"Замена стекла", "механический", 30, ServiceExtra::None},
        {"Реставрация механизма", "механический", 120, ServiceExtra::Disassembly},

        {"Диагностика", "хронограф", 20, ServiceExtra::None},
        {"Чистка и смазка хронографа", "хронограф", 100, ServiceExtra::Complications},
        {"Настройка хронографа", "хронограф", 70, ServiceExtra::Complications},
        {"Замена толкателей и кнопок", "хронограф", 50, ServiceExtra::None},
        {"Ремонт календаря и фазы луны", "хронограф", 80, ServiceExtra::Complications},
        {"Замена стекла", "хронограф", 35, ServiceExtra::None},
    };
    return list;
}

std::vector<const ServiceTemplate*> ServiceCatalog::forMechanism(const std::string& mechanismType) {
    std::vector<const ServiceTemplate*> result;
    for (const ServiceTemplate& t : all())
        if (t.mechanism == mechanismType) result.push_back(&t);
    return result;
}

namespace {
std::string lowerAscii(std::string s) {
    for (char& c : s)
        if (static_cast<unsigned char>(c) < 0x80) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
bool containsAny(const std::string& text, const std::vector<std::string>& words) {
    return std::any_of(words.begin(), words.end(),
                       [&](const std::string& w) { return text.find(w) != std::string::npos; });
}
} 

BrandInfo ServiceCatalog::brandInfo(const std::string& brand) {
    static const std::vector<std::string> premium = {"rolex", "omega", "patek", "audemars", "cartier",
        "breitling", "iwc", "panerai", "hublot", "jaeger", "tag heuer", "longines"};
    static const std::vector<std::string> middle = {"tissot", "seiko", "citizen", "orient", "hamilton",
        "rado", "fossil", "certina", "mido", "bulova"};
    std::string b = lowerAscii(brand);
    if (containsAny(b, premium)) return {"премиум", 1.75};
    if (containsAny(b, middle)) return {"средняя", 1.25};
    return {"обычная", 1.0};
}

std::shared_ptr<Service> ServiceCatalog::create(const ServiceTemplate& t, int complexity,
                                                int extraValue, double brandFactor) {
    double base = std::round(t.basePrice * brandFactor * 100.0) / 100.0;
    std::string name = t.name;
    int level = 0, functions = 0;
    switch (t.extra) {
        case ServiceExtra::None: break;
        case ServiceExtra::Disassembly: level = extraValue; break;
        case ServiceExtra::Complications: functions = extraValue; break;
    }

    if (t.mechanism == "кварцевый") return std::make_shared<QuartzRepairService>(name, base, complexity);
    if (t.mechanism == "механический") return std::make_shared<MechanicalRepairService>(name, base, complexity, level);
    return std::make_shared<ChronographRepairService>(name, base, complexity, functions);
}
