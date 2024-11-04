#pragma once
#include <string>
#include "pnl/pnl_vector.h"

class PricingResults
{
  private:
    double price;
    const PnlVect* delta;
    double priceStdDev;
    const PnlVect* deltaStdDev;
    double cpuTime;

  public:
    PricingResults(double price, double priceStdDev, const PnlVect* const delta, const PnlVect* const deltaStdDev, double cpuTime)
      : price(price)
      , priceStdDev(priceStdDev)
      , delta(delta)
      , deltaStdDev(deltaStdDev)
      , cpuTime(cpuTime)
    {
    }

    friend std::ostream& operator<<(std::ostream& stm, const PricingResults& res);
};
