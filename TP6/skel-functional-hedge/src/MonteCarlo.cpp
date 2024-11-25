#include <iostream>
#include <cstdlib>
#include <cstring>

#include "pnl/pnl_mathtools.h"
#include "MonteCarlo.hpp"

static void _set_diag_vect(PnlMat *M, const PnlVect *x)
{
    int i;
    for (i = 0; i < M->m; i++)
    {
        MLET(M, i, i) = GET(x, i);
    }
}

void MonteCarlo::prepare_delta(PnlVect *delta, PnlVect *std_devdelta)
{
    pnl_vect_resize(delta, mod->m_modelSize);
    pnl_vect_resize(std_devdelta, mod->m_modelSize);
    pnl_vect_set_zero(delta);
    pnl_vect_set_zero(std_devdelta);
}

void MonteCarlo::finalize_delta(PnlVect *delta, PnlVect *std_devdelta, const PnlVect *St, double t)
{
    pnl_vect_mult_double(delta, std::exp(-mod->m_interest * (opt->m_maturity - t)) / (m_nSamples * 2. * m_fdStep));
    pnl_vect_div_vect_term(delta, St);
    pnl_vect_mult_double(std_devdelta, std::exp(-2 * mod->m_interest * (opt->m_maturity - t)) / (m_nSamples * 4. * m_fdStep * m_fdStep));
    pnl_vect_div_vect_term(std_devdelta, St);
    pnl_vect_div_vect_term(std_devdelta, St);
    PnlVect *sqdelta = pnl_vect_copy(delta);
    pnl_vect_mult_vect_term(sqdelta, delta);
    pnl_vect_minus_vect(std_devdelta, sqdelta);
    pnl_vect_free(&sqdelta);
    pnl_vect_div_double(std_devdelta, double(m_nSamples));
    pnl_vect_map_inplace(std_devdelta, std::sqrt);
}

void MonteCarlo::finalize(double &price, double &std_dev, size_t nSamples, double t) const
{
    price = price * std::exp(-mod->m_interest * (opt->m_maturity - t)) / double(nSamples);
    std_dev = std::exp(-2 * mod->m_interest * (opt->m_maturity - t)) * std_dev / double(nSamples) - price * price;
    std_dev = std::sqrt(std_dev / double(nSamples));
}

MonteCarlo::MonteCarlo()
{
    rng = NULL;
    m_nSamples = 0;
    m_fdStep = 0.;
}

/**
 * Constructor
 * @param rng a PnlRng
 * @param P a hash map
 */
MonteCarlo::MonteCarlo(PnlRng *rng, const IParser &P)
{
    mod = instantiate_model(P);
    opt = instantiate_option(P);
    m_IsSize = mod->m_brownianSize;
    this->rng = rng;
    m_fdStep = 0.1;
    P.extract("sample number", m_nSamples);
    P.extract("FD step", m_fdStep, true);
    m_nSamplesSAA = m_nSamples;
    P.extract("sample number SAA", m_nSamplesSAA, true);
    if (m_nSamplesSAA > NUMBER_OF_SAMPLES_SAA_MAX)
        m_nSamplesSAA = NUMBER_OF_SAMPLES_SAA_MAX;
    m_BT = pnl_vect_new();
}

MonteCarlo::~MonteCarlo()
{
    if (mod != NULL)
        delete mod;
    if (opt != NULL)
        delete opt;
    pnl_vect_free(&m_BT);
}

void MonteCarlo::print(bool verbose) const
{
    std::cout << std::endl;
    std::cout << "*************************************" << std::endl;
    if (verbose)
    {
        mod->print();
        opt->print();
    }
    std::cout << " Number of Samples : " << m_nSamples << std::endl;
    std::cout << " Number of timesteps " << mod->m_nTimeSteps << std::endl;
    std::cout << " Number of Samples SAA : " << m_nSamplesSAA << std::endl;
    std::cout << " FD step : " << m_fdStep << std::endl;
    std::cout << "*************************************" << std::endl
              << std::endl;
}

/**
 * Computes the price by a crude Monte--Carlo estimator
 *
 * @param[out] prix price
 * @param[out] std_dev std_dev of the discounted payoff
 */
void MonteCarlo::price(double &prix, double &std_dev)
{
    double payoffVector;
    prix = 0.0;
    std_dev = 0.0;
    for (size_t i = 0; i < m_nSamples; i++)
    {
        mod->path(rng);
        payoffVector = opt->payoff(mod->m_pathMatrix);
        prix += payoffVector;
        std_dev += payoffVector * payoffVector;
    }
    finalize(prix, std_dev, m_nSamples);
}

/**
 * Compute the price and delta
 *
 * @param[out] prix price
 * @param[out] std_devprix std_dev of the discounted payoff
 * @param[out] delta delta of the option
 * @param[out] std_devdelta std_dev of the FD estimator
 */
void MonteCarlo::price_delta(double &prix, double &std_devprix, PnlVect *delta, PnlVect *std_devdelta)
{
    double payoffVector;
    prix = 0.0;
    std_devprix = 0.0;

    /* resize delta ... */
    prepare_delta(delta, std_devdelta);
    for (size_t i = 0; i < m_nSamples; i++)
    {
        mod->path(rng);
        payoffVector = opt->payoff(mod->m_pathMatrix);
        prix += payoffVector;
        std_devprix += payoffVector * payoffVector;

        for (int d = 0; d < mod->m_modelSize; d++)
        {
            double tmp_delta = 0.;
            mod->shiftPath(d, m_fdStep);
            tmp_delta += opt->payoff(mod->m_pathMatrix);
            mod->shiftPath(d, -2 * m_fdStep / (1 + m_fdStep));
            tmp_delta -= opt->payoff(mod->m_pathMatrix);
            mod->unshiftPath(d, -m_fdStep);
            LET(delta, d) += tmp_delta;
            LET(std_devdelta, d) += tmp_delta * tmp_delta;
        }
    }

    finalize(prix, std_devprix, m_nSamples);
    finalize_delta(delta, std_devdelta, mod->m_init);
}

/**
 * Compute the price by a crude Monte--Carlo estimator at time t
 *
 * @param t current date
 * @param past market values up to time t
 * @param[out] prix price
 * @param[out] std_dev std_dev of the discounted payoff
 */
void MonteCarlo::price(const PnlMat *past, double t, double &prix, double &std_dev)
{
    double payoffVector;
    prix = 0.0;
    std_dev = 0.0;
    for (size_t i = 0; i < m_nSamples; i++)
    {
        mod->path(past, t, rng);
        payoffVector = opt->payoff(mod->m_pathMatrix);
        prix += payoffVector;
        std_dev += payoffVector * payoffVector;
    }
    finalize(prix, std_dev, m_nSamples, t);
}

/**
 * Compute the price and delta at time t
 *
 * @param t current date
 * @param past market values up to time t
 * @param[out] prix price
 * @param[out] std_devprix std_dev of the discounted payoff
 * @param[out] delta delta of the option
 * @param[out] std_devdelta std_dev of the FD estimator
 */
void MonteCarlo::price_delta(const PnlMat *past, double t, double &prix, double &std_devprix, PnlVect *delta, PnlVect *std_devdelta)
{
    double payoffVector;
    prix = 0.0;
    std_devprix = 0.0;

    /* resize delta ... */
    prepare_delta(delta, std_devdelta);
    for (size_t i = 0; i < m_nSamples; i++)
    {
        mod->path(past, t, rng);
        payoffVector = opt->payoff(mod->m_pathMatrix);
        prix += payoffVector;
        std_devprix += payoffVector * payoffVector;

        for (int d = 0; d < mod->m_modelSize; d++)
        {
            double tmp_delta = 0.;
            mod->shiftPath(d, m_fdStep, t);
            tmp_delta += opt->payoff(mod->m_pathMatrix);
            mod->shiftPath(d, -2 * m_fdStep / (1 + m_fdStep), t);
            tmp_delta -= opt->payoff(mod->m_pathMatrix);
            mod->unshiftPath(d, -m_fdStep, t);
            LET(delta, d) += tmp_delta;
            LET(std_devdelta, d) += tmp_delta * tmp_delta;
        }
    }

    PnlVect St = pnl_vect_wrap_mat_row(past, past->m - 1);
    finalize(prix, std_devprix, m_nSamples, t);
    finalize_delta(delta, std_devdelta, &St, t);
}

double MonteCarlo::hedge(const PnlMat *market, bool verbose)
{
    double price, std_dev_price;
    PnlVect *dprice, *dprice_prev, *std_dev_dprice;
    dprice = pnl_vect_create(mod->m_modelSize);
    dprice_prev = pnl_vect_create(mod->m_modelSize);
    std_dev_dprice = pnl_vect_create(mod->m_modelSize);
    double deltaTau = mod->m_maturity / double(mod->m_nHedgingDates);
    double riskFreePortfolio = 0;
    pnl_vect_set_zero(dprice_prev);
    PnlMat *subMarket = pnl_mat_create(mod->m_nTimeSteps, mod->m_modelSize);
    // It is essential to perform this resize and not to directly create the matrix with the correct
    // size. We use a side effect of pnl_mat_resize.
    pnl_mat_resize(subMarket, 1, mod->m_modelSize);
    int subMarketIndex = 0;
    int H_N = mod->m_nHedgingDates / mod->m_nTimeSteps;
    for (int i = 0; i < market->m; i++)
    {
        double tau_i = i * deltaTau;
        PnlVect Stau_i = pnl_vect_wrap_mat_row(market, i);
        // Warning: il faut extraire les bonnes lignes de market!!!
        if (i % H_N == 1)
        {
            subMarketIndex++;
            pnl_mat_resize(subMarket, subMarketIndex + 1, mod->m_modelSize);
        }
        pnl_mat_set_row(subMarket, &Stau_i, subMarketIndex);
        price_delta(subMarket, tau_i, price, std_dev_price, dprice, std_dev_dprice);
        riskFreePortfolio = riskFreePortfolio * std::exp(mod->m_interest * deltaTau) - pnl_vect_scalar_prod(dprice, &Stau_i) + pnl_vect_scalar_prod(dprice_prev, &Stau_i);
        if (i == 0)
            riskFreePortfolio += price;
        pnl_vect_clone(dprice_prev, dprice);
        if (verbose)
        {
            std::cout << "Error at time " << tau_i << ": " << riskFreePortfolio - price + pnl_vect_scalar_prod(dprice, &Stau_i) << " (option price " << price << ")" << std::endl;
        }
    }

    PnlVect ST = pnl_vect_wrap_mat_row(market, market->m - 1);
    double pl = riskFreePortfolio - price + pnl_vect_scalar_prod(dprice, &ST);
    pnl_vect_free(&dprice);
    pnl_vect_free(&std_dev_dprice);
    return pl;
}

//
// Sample averaging approach
//

/**
 * Computes the different terms involved in the sample average minimization
 * in a way to reduce to computational cost
 *  (same drift vector for all time steps)
 *
 * \f[
 * E(GG^\prime \phi(G)^2 e^{-\theta \cdot G} )
 * \f]
 *
 * \f[
 * E(G  \phi(G)^2 e^{-\theta \cdot G})
 * \f]
 *
 * \f[
 * E( \phi(G)^2 e^{-\theta \cdot G})
 * \f]
 *

 * @param[in] g vectors of the final values of W_T
 * @param[in] theta current value of the drift vector
 * @param[in] payoffs vector of the payoffs computed on the paths built with
 * the vector sample_G
 * @param[out] expect_0
 * @param[out] expect_1
 * @param[out] expect_2
 */
void MonteCarlo::expectation_order_n(PnlVect *const *g, const PnlVect *theta, const PnlVect *payoffs,
                                     double &expect_0, PnlVect *expect_1, PnlMat *expect_2)
{
    double tmp;
    expect_0 = 0.0;
    pnl_vect_resize(expect_1, theta->size);
    pnl_mat_resize(expect_2, theta->size, theta->size);
    pnl_vect_set_zero(expect_1);
    pnl_mat_set_zero(expect_2);

    for (size_t i = 0; i < m_nSamplesSAA; i++)
    {
        tmp = SQR(GET(payoffs, i)) * std::exp(-(pnl_vect_scalar_prod(theta, g[i])));
        expect_0 += tmp;

        pnl_vect_axpby(tmp, g[i], 1., expect_1); /* E1 += tmp  * g[i] */
        pnl_mat_dger(tmp, g[i], g[i], expect_2); /* E2 += tmp  * g[i]' * g[i] */
    }
}

/**
 * Computes the optimal drift (one drift vector for all time steps) using the
 * random variables sample_G
 *
 * @param[in] theta drift vector (one for all time steps)
 * @param[in] noverbose if true nothing is printed about the intermediate steps
 * of the Newton procedure
 *
 * @return the optimal drift vector
 */
void MonteCarlo::sample_averaging_newton(PnlVect *theta, bool noverbose)
{
    double expect_0, norm_gradv;
    PnlVect *payoffs = pnl_vect_create(m_nSamplesSAA);
    PnlVect **G = new PnlVect *[m_nSamplesSAA];
    PnlVect *expect_1 = pnl_vect_new();
    PnlVect *gradVector = pnl_vect_new();
    PnlMat *expect_2 = pnl_mat_new();
    PnlMat *hesVector = pnl_mat_create(m_IsSize, m_IsSize);
    double EPS = 0.00000001 * m_IsSize;
    int k = 30;
    double constant_mode = getConstant(); // maturity or 1 dependending on Full, Reduced.
    pnl_vect_resize(theta, m_IsSize);
    pnl_vect_set_zero(theta);

    for (size_t i = 0; i < m_nSamplesSAA; i++)
    {
        mod->path(rng);
        LET(payoffs, i) = opt->payoff(mod->m_pathMatrix);
        G[i] = getGaussianIsVariable(mod->m_deltaB);
    }

    for (int l = 0; l < k; l++)
    {
        expectation_order_n(G, theta, payoffs, expect_0, expect_1, expect_2);
        pnl_vect_clone(gradVector, theta);
        pnl_vect_axpby(1. / (-constant_mode * expect_0), expect_1, 1., gradVector);

        /* hesVector = I + ( E2 E 0 + E1'E1) / (E0^2 maturity) */
        pnl_mat_div_double(expect_2, expect_0 * constant_mode);
        pnl_mat_set_id(hesVector);
        pnl_mat_plus_mat(hesVector, expect_2);
        pnl_mat_dger(-1. / (expect_0 * expect_0 * constant_mode), expect_1, expect_1, hesVector);

        norm_gradv = pnl_vect_norm_two(gradVector);
        pnl_mat_chol(hesVector);
        pnl_mat_chol_syslin_inplace(hesVector, gradVector);
        pnl_vect_axpby(-1., gradVector, 1., theta); /* theta -= gradVector */

        if (!noverbose)
            std::cout << "iteration " << l << ", norm of the gradient : " << norm_gradv << std::endl;

        if (!noverbose && norm_gradv < EPS)
        {
            std::cout << "iteration : " << l << std::endl;
            break;
        }
    }
    for (size_t i = 0; i < m_nSamplesSAA; i++)
        pnl_vect_free(&(G[i]));
    delete[] G;

    pnl_vect_free(&payoffs);
    pnl_vect_free(&expect_1);
    pnl_vect_free(&gradVector);
    pnl_mat_free(&expect_2);
    pnl_mat_free(&hesVector);
}

/**
 * Computes the price using a sample average based IS estimator
 *
 * @param[out] theta computed drift
 * @param[out] price estimated price
 * @param[out] std_dev asymptotic std_dev of the estimator
 * @param[in] noverbose
 */
void MonteCarlo::price_SAA(PnlVect *theta, double &price, double &std_dev, bool noverbose)
{
    double tmp;
    price = 0.0;
    std_dev = 0.0;

    /* computation of the theta optimal */
    sample_averaging_newton(theta, noverbose);

    /* computation of MC with that value using independent samples */
    for (size_t i = 0; i < m_nSamples; i++)
    {
        mod->path(rng, theta);
        tmp = opt->payoff(mod->m_pathMatrix) * gaussianWeight(mod->m_deltaB, theta);
        price += tmp;
        std_dev += tmp * tmp;
    }
    finalize(price, std_dev, m_nSamples);
}

/**
 * Computes the price and deltas using a sample average based IS estimator (same
 * drift vector for all time steps)
 *
 * @param[out] theta computed drift
 * @param[out] price estimated price
 * @param[out] std_dev asymptotic std_dev of the estimator
 * @param[out] delta vector of the deltas
 * @param[out] std_devdelta std_dev of the deltas
 * @param[in] noverbose
 */
void MonteCarlo::price_delta_SAA(PnlVect *theta, double &price, double &std_dev, PnlVect *delta, PnlVect *std_devdelta, bool noverbose)
{
    double tmp;
    price = 0.0;
    std_dev = 0.0;

    /* computation of the theta optimal */
    sample_averaging_newton(theta, noverbose);

    /* computation of MC with that value using the same samples */
    /* resize delta ... */
    prepare_delta(delta, std_devdelta);

    for (size_t i = 0; i < m_nSamples; i++)
    {
        mod->path(rng, theta);
        tmp = opt->payoff(mod->m_pathMatrix) * gaussianWeight(mod->m_deltaB, theta, false);
        price += tmp;
        std_dev += tmp * tmp;

        for (int d = 0; d < mod->m_modelSize; d++)
        {
            double tmp_delta = 0.;
            double payoff_val = 0.;
            mod->shiftPath(d, m_fdStep);
            payoff_val = opt->payoff(mod->m_pathMatrix);
            mod->unshiftPath(d, m_fdStep);

            mod->shiftPath(d, -m_fdStep);
            payoff_val -= opt->payoff(mod->m_pathMatrix);
            tmp_delta = payoff_val * gaussianWeight(mod->m_deltaB, theta, false);
            mod->unshiftPath(d, -m_fdStep);

            LET(delta, d) += tmp_delta;
            LET(std_devdelta, d) += tmp_delta * tmp_delta;
        }
    }

    finalize(price, std_dev, m_nSamples);
    finalize_delta(delta, std_devdelta, mod->m_init);
}

double MonteCarlo::gaussianWeight(const PnlMat *G, const PnlVect *x, bool isplus)
{
    pnl_mat_sum_vect(m_BT, G, 'r');
    double scalar = pnl_vect_scalar_prod(m_BT, x) * mod->m_sqrt_dt;
    double norm = pnl_vect_scalar_prod(x, x) * mod->m_maturity;
    if (isplus)
        return exp(-scalar + norm / 2.);
    else
        return exp(-scalar - norm / 2);
}

void MonteCarlo::gaussianGradWeight(PnlVect *res, const PnlMat *G, const PnlVect *x)
{
    pnl_mat_sum_vect(m_BT, G, 'r');
    pnl_vect_div_double(m_BT, mod->m_sqrt_nTimeSteps);
    pnl_vect_clone(res, m_BT);
    pnl_vect_mult_double(res, -1. / sqrt(mod->m_maturity) * exp(-2 * (pnl_vect_scalar_prod(m_BT, x))*sqrt(mod->m_maturity) - pnl_vect_scalar_prod(x, x) * mod->m_maturity));
}

PnlVect *MonteCarlo::getGaussianIsVariable(const PnlMat *G) const
{
    PnlVect *g = pnl_vect_new();
    pnl_mat_sum_vect(g, G, 'r');
    pnl_vect_div_double(g, mod->m_sqrt_nTimeSteps);
    return g;
}
