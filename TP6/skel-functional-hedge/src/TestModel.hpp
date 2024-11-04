#ifndef _TEST_MODEL_H
#define _TEST_MODEL_H

#include "Model.hpp"
#include "pnl/pnl_matrix.h"
#include "pnl/pnl_random.h"

class TestModel : public Model
{
  public:
    PnlVect *m_sigma;
    TestModel();
    TestModel(const IParser &P);
    ~TestModel();
    void print() const;
    virtual void path(const PnlVect *St, double t, int last_index, double dt, PnlVect *drift, PnlMat *G, PnlMat *sample);
};

#endif
