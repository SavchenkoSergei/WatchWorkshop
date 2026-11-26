#pragma once
#include <iosfwd>
#include <memory>
#include <string>

class Service {
public:
    Service(std::string name, double baseCost, int complexity);
    virtual ~Service() = default;

    virtual double calculateCost() const = 0;
    virtual int calculateDuration() const = 0;
    virtual void write(std::ostream& os) const = 0;
    static std::shared_ptr<Service> read(std::istream& is);

    const std::string& getName() const { return name_; }
    double getBaseCost() const { return baseCost_; }
    int getComplexity() const { return complexity_; }

protected:
    static constexpr double kComplexityRate = 0.2;

    double baseWithComplexity() const { return baseCost_ * (1.0 + kComplexityRate * complexity_); }
    void writeCommon(std::ostream& os, const char* typeTag) const;

private:
    std::string name_;
    double baseCost_;
    int complexity_;
};