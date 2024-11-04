#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstring>

using namespace std;

#include "TestOption.hpp"
#include "jlparser/parser.hpp"
#include "pnl/pnl_vector.h"

//
// Basket options
//

double TestOption::payoff(const PnlMat *path_val)
{
    return 1.;
}

TestOption::TestOption() {}

TestOption::TestOption(const IParser &P) : Option(P)
{
    m_label = "test";
}

void TestOption::print() const
{
    cout << "**** Test Option Characteristics ****" << endl;
    Option::print();
}

TestOption::~TestOption() {}
