#include <iostream>
#include "pnl/pnl_vector.h"
#include "pnl/pnl_random.h"
#include "math.h"

void simulation_of_brownien(PnlVect *res, double T, int N, PnlRng *rng);

int main()
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, time(NULL));

    const double T = 1.0;
    const int N = 100;

    PnlVect *res_of_sim_brow = pnl_vect_create(N + 1);
    simulation_of_brownien(res_of_sim_brow, T, N, rng);

    pnl_vect_print(res_of_sim_brow);

    pnl_rng_free(&rng);
    pnl_vect_free(&res_of_sim_brow);
    return 0;
}

void simulation_of_brownien(PnlVect *res, double T, int N, PnlRng *rng)
{
    pnl_vect_set(res, 0, 0.0); // X_0 = 0

    for (int i = 1; i <= N; i++)
    {

        pnl_vect_set(res, i, pnl_vect_get(res, i - 1) + std::sqrt(T / (double)N) * pnl_rng_normal(rng));
    }
}
