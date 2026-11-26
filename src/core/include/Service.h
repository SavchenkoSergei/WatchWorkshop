#pragma once
// Абстрактная ремонтная услуга. Потомки реализуют расчёт стоимости и срока.
//   стоимость: C = B * (1 + k * S) + X     (B - цена работы, S - сложность 1..5)
//   срок:      T = S + Y                   (в рабочих днях)
// X и Y зависят от вида услуги (см. потомков).
#include <iosfwd>
#include <memory>
#include <string>

class Service {
public:
    Service(std::string name, double baseCost, int complexity);
    virtual ~Service() = default;

    virtual double calculateCost() const = 0;
    virtual int calculateDuration() const = 0;
    // Запись услуги в поток (первым пишется тип услуги).
    virtual void write(std::ostream& os) const = 0;
    // Фабрика: читает услугу из потока; при ошибке возвращает nullptr.
    static std::shared_ptr<Service> read(std::istream& is);

    const std::string& getName() const { return name; }
    double getBaseCost() const { return baseCost; }
    int getComplexity() const { return complexity; }

protected:
    static constexpr double complexityRate = 0.2;   // коэффициент k

    // B * (1 + k * S) - часть стоимости, общая для всех видов услуг
    double baseWithComplexity() const { return baseCost * (1.0 + complexityRate * complexity); }
    void writeCommon(std::ostream& os, const char* typeTag) const;

    std::string name;
    double baseCost;
    int complexity;
};
