#include <iostream>
#include "PricingResults.hpp"
#include "jlparser/json_helper.hpp"

std::ostream&
operator<<(std::ostream& stm, const PricingResults& res)
{

    nlohmann::json j = {
        {"price", res.price},
        {"priceStdDev", res.priceStdDev},
        {"delta", res.delta},
        {"deltaStdDev", res.deltaStdDev},
        {"time", res.cpuTime},
    };
    stm << std::setw(4) << j << std::endl;
    return stm;
}