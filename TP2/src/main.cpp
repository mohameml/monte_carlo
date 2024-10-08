#include <iostream>
#include "pnl/pnl_vector.h"
#include "pnl/pnl_random.h"
#include <cmath>

//  Constante globale
double S_0 = 100.0;
double K = 0.95;
double r = 0.02;
double sigma = 0.25;
double T = 2.0;
int N = 24;

double price(PnlRng *rng);

int main()
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, time(NULL));

    double price_0 = price(rng);
    std::cout << "price_0 = " << price_0 << std::endl;

    pnl_rng_free(&rng);
    return 0;
}

void simulation_s_t(PnlVect *res, PnlRng *rng)
{

    double dt = T / (double)N;
    pnl_vect_set(res, 0, S_0);
    for (int i = 1; i <= N; i++)
    {
        double s_t = pnl_vect_get(res, i - 1) * std::exp((r - std::pow(sigma, 2) / 2) * dt + sigma * std::sqrt(dt) * pnl_rng_normal(rng));
        pnl_vect_set(res, i, s_t);
    }
}

double payoff(PnlVect *path)
{
    double S_T = pnl_vect_get(path, path->size - 1);

    // find the max :
    double max_ = 0.0;

    for (int i = 0; i < path->size; i++)
    {
        double s_t = pnl_vect_get(path, i);
        if (s_t > max_)
        {
            max_ = s_t;
        }
    }
    double a = max_ - K * S_T;
    double res = a > 0 ? a : 0;

    return res;
}

double price(PnlRng *rng)
{
    double price_0 = 0.0;
    int M = 50000;

    PnlVect *sim = pnl_vect_create(N + 1);
    for (int i = 0; i < M; i++)
    {
        simulation_s_t(sim, rng);
        price_0 += payoff(sim);
    }

    pnl_vect_free(&sim);

    price_0 = std::exp(-r * T) * price_0 * (1.0 / (double)M);

    return price_0;
}