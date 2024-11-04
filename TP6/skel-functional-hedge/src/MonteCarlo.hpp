#ifndef _MONTECARLO_H
#define _MONTECARLO_H

#include "Model.hpp"
#include "Option.hpp"
#include "Martingale.hpp"
#include "pnl/pnl_matrix.h"
#include "pnl/pnl_random.h"
#include "jlparser/parser.hpp"

#define NUMBER_OF_SAMPLES_SAA_MAX 5E5

/**
 * Monte Carlo class.
 */
class MonteCarlo
{
public:
    Model *mod;
    Option *opt;
    int m_IsSize;
    PnlRng *rng;
    double m_fdStep; /*!< Finite difference step size */
    size_t m_nSamples; /*!< number of simulations */

    MonteCarlo();
    MonteCarlo(PnlRng *, const IParser &ParamTab);
    ~MonteCarlo();


    void print(bool verbose = false) const;

    void price(double &prix, double &stdDev);
    void price_delta(double &prix, double &stdDevPrix, PnlVect *delta, PnlVect *stdDevDelta);
    void price(const PnlMat *past, double t, double &prix, double &stdDev);
    void price_delta(const PnlMat *past, double t, double &prix, double &stdDevPrix, PnlVect *delta, PnlVect *stdDevDelta);

    void price_SAA(PnlVect *theta, double &price, double &stdDev, bool noverbose = false);
    void price_delta_SAA(PnlVect *theta, double &price, double &stdDev, PnlVect *delta, PnlVect *stdDevDelta, bool noverbose = false);

    double hedge(const PnlMat *market, bool verbose = true);

protected:
    /**
     * Reduce the price and the variance
     *
     * @param[in, out] price
     * @param[in, out] stdDev On output, it contains the variance of the estimator
     * @param nSamples number of samples used in the estimator
     * @param t current date
     */
    void finalize(double &price, double &stdDev, size_t nSamples, double t = 0) const;
    /**
     * Resize and set to zero
     *
     * @param[in, out] delta vector
     * @param[in, out] vardelta vector
     */
    void prepare_delta(PnlVect *delta, PnlVect *vardelta);
    /**
     * Finalize the computation of the delta's and vardelta's
     *
     * @param[in, out] delta vector
     * @param[in, out] vardelta vector
     * @param St current value of the asset
     * @param t current date
     */
    void finalize_delta(PnlVect *delta, PnlVect *vardelta, const PnlVect *St, double t = 0);
    /**
     * Computes  martingale weight corresponding the Cameron-Martin theorem
     *
     * @param G matrix of the Gaussian m_interest.v. used to build the Brownian
     * @param x is the drift vector
     * @param isplus gives the sign within the exponential
     *    if isplus == true, return exp(-theta.G+|theta|^2/2)
     *    if isplus == false, return exp(-theta.G-|theta|^2/2)
     *
     * @return exp( -x . g sqrt(maturity) - x^2 * maturity / 2)
     */
    double gaussianWeight(const PnlMat *G, const PnlVect *x, bool isplus = false);

    /**
     * Computes the gradient of the variance in the case of the Cameron Martin
     * theorem
     *
     * @param[out] res holds the gradient of the Girsanov payoff on exit
     * @param G holds the increments of the Brownian path
     * @param x is the drift vector
     *
     * @return -g / sqrt(maturity) *  exp (-2 g . x * sqrt(maturity) - x^2 * maturity)
     */
    void gaussianGradWeight(PnlVect *res, const PnlMat *G, const PnlVect *x);

    /**
     * Return the random vector involved in Girsanov's weight
     *
     * @param G Matrix of renormalized brownian increments
     *
     * @return B_T
     */
    PnlVect *getGaussianIsVariable(const PnlMat *G) const;

    void expectation_order_n(PnlVect *const *g, const PnlVect *theta, const PnlVect *payoffs, double &expect_0, PnlVect *expect_1, PnlMat *expect_2);
    void sample_averaging_newton(PnlVect *theta, bool noverbose);

    inline double getConstant() const
    {
        return mod->m_maturity;
    }

    size_t m_nSamplesSAA;
    PnlVect *m_BT;
};

#endif
