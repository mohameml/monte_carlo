/* Nothing is printed to files if the number of samples
 * execeeds 100000. Max File size is exceeded otherwise. */
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <string>

#include "Option.hpp"
#include "TestOption.hpp"
#include "AsianOption.hpp"
#include "BarrierOption.hpp"
#include "BasketOption.hpp"
#include "MountainOption.hpp"
#include "MaxOption.hpp"

using namespace std;

Option::Option() {}

/**
 * Creates the correct option according to name
 */
Option *instantiate_option(const IParser &P)
{
    Option *opt = NULL;
    string optionType;
    P.extract("option type", optionType);

    if (optionType == "test")
        opt = new TestOption(P);
    else if (optionType == "exchange" || optionType == "basket")
        opt = new BasketOption(P);
    else if (optionType == "geometriccall")
        opt = new GeometricBasketOption(P, true);
    else if (optionType == "geometricput")
        opt = new GeometricBasketOption(P, false);
    else if (optionType == "atlas")
        opt = new AtlasOption(P);
    else if (optionType == "altiplano")
        opt = new AltiplanoOption(P);
    else if (optionType == "everest")
        opt = new EverestOption(P);
    else if (optionType == "digital")
        opt = new DigitalOption(P);
    else if (optionType == "digitalfinal")
        opt = new DigitalFinalOption(P);
    else if (optionType == "digitalbasket")
        opt = new DigitalBasketOption(P);
    else if (optionType == "barrier")
        opt = new BarrierOption(P);
    else if (optionType == "asian")
        opt = new AsianCallOption(P);
    else if (optionType == "maximum")
        opt = new MaxOption(P);
    else if (optionType == "bestof")
        opt = new BestOfOption(P);
    else if (optionType == "worstof")
        opt = new WorstOfOption(P);
    else if (optionType == "performance")
        opt = new PerformanceOption(P);
    else {
        cout << "Option " << optionType << " unknow. Abort." << endl;
        abort();
    }

    return opt;
}

Option::Option(const IParser &P)
{
    if (!P.extract("model size", m_optionSize, true)) {
        P.extract("option size", m_optionSize);
    }
    if (! P.extract("timestep number", m_nTimeSteps, true)) {
        P.extract("fixing dates number", m_nTimeSteps);
    }
    P.extract("maturity", m_maturity);
}

void Option::print() const
{
    cout << " model size : " << m_optionSize << endl;
    cout << " m_maturity time : " << m_maturity << endl;
}
