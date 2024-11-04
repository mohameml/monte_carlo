#pragma once
#include <iostream>

class HedgingResults
{
  private:
    double initialPrice;
    double initialPriceStdDev;
    double finalPnL;
    double cpuTime;

  public:
    HedgingResults(double initialPrice, double initialPriceStdDev, double finalPnL, double cpuTime)
      : initialPrice(initialPrice)
      , initialPriceStdDev(initialPriceStdDev)
      , finalPnL(finalPnL)
      , cpuTime(cpuTime)
    {
    }

    friend std::ostream& operator<<(std::ostream& stm, const HedgingResults& res);
};
