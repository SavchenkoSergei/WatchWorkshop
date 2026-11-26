#pragma once
#include "Service.h"

// Ремонт хронографов.
//   X = U * N  (N - число дополнительных функций: календарь, фаза луны, будильник,
//               репетир и т. п.; U - надбавка за одну функцию), Y = N
class ChronographRepairService : public Service {
public:
    ChronographRepairService(std::string name, double baseCost, int complexity, int complicationCount);
    double calculateCost() const override;
    int calculateDuration() const override;
    void write(std::ostream& os) const override;
    int getComplicationCount() const { return complicationCount; }
private:
    static constexpr double complicationPrice = 40.0;   // U, BYN
    int complicationCount;
};
