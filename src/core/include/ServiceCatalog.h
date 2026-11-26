#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Service.h"

enum class ServiceExtra { None, Disassembly, Complications };

struct ServiceTemplate {
    std::string name;
    std::string mechanism;    
    double basePrice;         
    ServiceExtra extra;
};

struct BrandInfo {
    std::string title;        
    double factor;             
};

class ServiceCatalog {
public:
    static const std::vector<ServiceTemplate>& all();
    static std::vector<const ServiceTemplate*> forMechanism(const std::string& mechanismType);
    static BrandInfo brandInfo(const std::string& brand);

    static std::shared_ptr<Service> create(const ServiceTemplate& t, int complexity, int extraValue,
                                           double brandFactor);
};
