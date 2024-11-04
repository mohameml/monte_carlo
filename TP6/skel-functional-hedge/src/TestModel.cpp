#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;

#include "TestModel.hpp"
#include "jlparser/parser.hpp"
#include "pnl/pnl_matrix.h"

TestModel::TestModel() : Model()
{
}

TestModel::~TestModel()
{
    if (m_sigma)
        pnl_vect_free(&m_sigma);
}

TestModel::TestModel(const IParser &P)
    : Model(P)
{
    P.extract("volatility", m_sigma, m_modelSize);
}

/**
 * Computes one path of the model using m_deltaB_drift
 */
void TestModel::path(const PnlVect *St, double t, int last_index, double dt, PnlVect *drift, PnlMat *G, PnlMat *sample)
{
    pnl_mat_set_double(sample, 1.);
}

void TestModel::print() const
{
    cout << "**** Test Model Characteristics ****" << endl;
    Model::print();
}
