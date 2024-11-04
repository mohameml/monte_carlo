#ifndef _TEST_OPTION_H
#define _TEST_OPTION_H

#include "pnl/pnl_matrix.h"
#include "jlparser/parser.hpp"
#include "Option.hpp"

class TestOption : public Option
{
  public:
    TestOption();
    TestOption(const IParser &ParamTab);
    ~TestOption();
    void print() const;

    double payoff(const PnlMat *path_val);
};

#endif
