#include <iostream>
#include "pnl/pnl_vector.h"
#include "pnl/pnl_random.h"
#include "math.h"
#include "pnl/pnl_specfun.h"
#include <string>
#include <fstream>

void simulation_of_brownien(PnlVect *res, double T, int N, PnlRng *rng);
double estimation_taux_a(double T, int N, PnlRng *rng, int M, double a);
double calcul_taux_a_theorique(double a, double T);
void test_a_1(PnlRng *rng);
void generata_file_biais(const std::string &file_name, int nbs, PnlRng *rng);
void simulation_of_brownien_near_to_a(PnlVect *res, double T, int N, PnlRng *rng, double a, double epsilon, PnlVect *sim_b);
double estimation_taux_a_refine(double T, int N, PnlRng *rng, int M, double a, double epsilon);
void generata_file_biais_refine(const std::string &file_name, int nbs, PnlRng *rng);

int main()
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, time(NULL));

    double a = 2.0;
    int M = 1000;

    test_a_1(rng);
    // generata_file_biais("tests/data.txt", 100, rng);
    // generata_file_biais_refine("tests/data_refine.txt", 100, rng);

    pnl_rng_free(&rng);
    return 0;
}

void simulation_of_brownien(PnlVect *res, double T, int N, PnlRng *rng)
{
    pnl_vect_set(res, 0, 0.0);

    for (int i = 1; i <= N; i++)
    {

        pnl_vect_set(res, i, pnl_vect_get(res, i - 1) + std::sqrt(T / (double)N) * pnl_rng_normal(rng));
    }
}

double estimation_taux_a(double T, int N, PnlRng *rng, int M, double a)
{

    double taux_a = 0.0;

    double setp = T / (double)N;

    PnlVect *sim_brow = pnl_vect_create(N + 1);

    for (int i = 0; i < M; i++)
    {
        simulation_of_brownien(sim_brow, T, N, rng);

        for (int j = 0; j < N + 1; j++)
        {
            double w_t = pnl_vect_get(sim_brow, j);
            // fabs(w_t - a) <= pow(10, -2)
            if (w_t >= a || j == N)
            {
                taux_a += j * setp;
                break;
            }
        }
    }

    taux_a = taux_a * (1 / (double)M);

    pnl_vect_free(&sim_brow);
    return taux_a;
}

double calcul_taux_a_theorique(double a, double T)
{

    double gamma_plus = pnl_sf_gamma_inc(1.0 / 2.0, a * a / (2.0 * T));
    double gamma_moins = pnl_sf_gamma_inc(-1.0 / 2.0, a * a / (2.0 * T));
    double inv_sqrt_PI = M_2_SQRTPI / 2;
    double res = T * (1 - gamma_plus * inv_sqrt_PI) + (a * a * inv_sqrt_PI / 2.0) * gamma_moins;

    return res;
}

void test_a_1(PnlRng *rng)
{

    double a = 1.0;
    double T = 3.0;
    double M = 500000;

    // double taux_a_estimation = estimation_taux_a(T, 1000, rng, M, a);
    // double taux_a_theorique = calcul_taux_a_theorique(a, T);
    // std::cout << "================ N = " << 1000 << " =======================" << std::endl;
    // std::cout << "taux_a_estimation = " << taux_a_estimation << std::endl;
    // std::cout << "taux_a_theorique = " << taux_a_theorique << std::endl;

    for (int i = 10; i < 1000; i += 100)
    {

        double taux_a_estimation = estimation_taux_a(T, i, rng, M, a);
        double taux_a_theorique = calcul_taux_a_theorique(a, T);
        std::cout << "================ N = " << i << " =======================" << std::endl;
        std::cout << "taux_a_estimation = " << taux_a_estimation << std::endl;
        std::cout << "taux_a_theorique = " << taux_a_theorique << std::endl;
    }
}

void generata_file_biais(const std::string &file_name, int nbs, PnlRng *rng)
{
    double a = 1.0;
    double T = 3.0;
    int M = 500000;

    std::ofstream file(file_name.c_str());

    if (!file.is_open())
    {
        exit(1);
    }

    double taux_a_theorique = calcul_taux_a_theorique(a, T);

    for (int i = 1; i < nbs; i++)
    {
        double taux_a_estimation = estimation_taux_a(T, i, rng, M, a);

        file << std::log(i) << "," << std::log(taux_a_estimation - taux_a_theorique) << "\n";
    }

    file.close();
}

void simulation_of_brownien_near_to_a(PnlVect *res, double T, int N, PnlRng *rng, double a, double epsilon, PnlVect *sim_b)
{

    for (int i = 0; i < sim_b->size; i++)
    {
        double w_s = pnl_vect_get(sim_b, i);
        if (fabs(w_s - a) < epsilon)
        {
            pnl_vect_set(res, 0, w_s);
            for (int j = 1; j <= N; j++)
            {
                double w_t_j = pnl_vect_get(res, j - 1) + std::sqrt(T / (double)N) * pnl_rng_normal(rng);
                pnl_vect_set(res, j, w_t_j);
            }

            break;
        }
    }
}

double estimation_taux_a_refine(double T, int N, PnlRng *rng, int M, double a, double epsilon)
{

    double taux_a = 0.0;

    double setp = T / (double)N;

    PnlVect *sim_brow = pnl_vect_create(N + 1);
    PnlVect *sim_brow_near_a = pnl_vect_create(N + 1);

    for (int i = 0; i < M; i++)
    {
        simulation_of_brownien(sim_brow, T, N, rng);
        simulation_of_brownien_near_to_a(sim_brow_near_a, T, N, rng, a, epsilon, sim_brow);

        for (int j = 0; j < N + 1; j++)
        {
            double w_t = pnl_vect_get(sim_brow_near_a, j);
            // fabs(w_t - a) <= pow(10, -2)
            if (w_t >= a)
            {
                taux_a += std::min(j * setp, T);
                break;
            }
        }
    }

    taux_a = taux_a * (1 / (double)M);

    pnl_vect_free(&sim_brow);
    pnl_vect_free(&sim_brow_near_a);
    return taux_a;
}

void generata_file_biais_refine(const std::string &file_name, int nbs, PnlRng *rng)
{
    double a = 1.0;
    double T = 3.0;
    int M = 500000;
    double epsilon = pow(10, -1);

    std::ofstream file(file_name.c_str());

    if (!file.is_open())
    {
        exit(1);
    }

    double taux_a_theorique = calcul_taux_a_theorique(a, T);

    for (int i = 1; i < nbs; i++)
    {
        double taux_a_estimation = estimation_taux_a_refine(T, i, rng, M, a, epsilon);

        file << std::log(i) << "," << std::log(fabs(taux_a_estimation - taux_a_theorique)) << "\n";
    }

    file.close();
}