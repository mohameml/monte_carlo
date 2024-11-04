#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdio>

#include "Model.hpp"
#include "Option.hpp"
#include "jlparser/parser.hpp"
#include "MonteCarlo.hpp"
#include "compare.hpp"

#include "cxxopts.hpp"

using namespace std;

/**
 * Run the right MonteCarlo routine
 *
 * @param rng PnlRng
 * @param map an instance of IParser
 * @param delta a boolean to decide whether to compute the delta
 * @param sample_average a boolean to decide whether to use IS with sample
 * averaging.
 * @param seed an integer to reset the ssed of the random number generator
 * @param price a boolean to tell to only compute the price
 * @param verbose a boolean to activate message printing
 * parameter
 */
void MonteCarloWrapper(PnlRng *rng, const IParser &map, int seed, bool price, bool delta,
                       bool sample_average, bool verbose)
{
    PnlVect *drift, *dprice, *dpricestd_dev;
    clock_t begin, end;
    int nb_traj = 10000;

    drift = pnl_vect_new();
    dprice = pnl_vect_new();
    dpricestd_dev = pnl_vect_new();

    MonteCarlo mc(rng, map);
    mc.print(verbose);

    if (price) {
        double price, std_dev;
        begin = clock();
        if (delta) {
            mc.price_delta(price, std_dev, dprice, dpricestd_dev);
            printf("mc price: %f\n", price);
            printf("price std_dev: %f\n", std_dev);
            printf("mc delta: ");
            pnl_vect_print_asrow(dprice);
            printf("std_dev delta: ");
            pnl_vect_print_asrow(dpricestd_dev);
        } else {
            mc.price(price, std_dev);
            printf("mc price: %f\n", price);
            printf("price std_dev: %f\n", std_dev);
        }

        end = clock();
        printf("CPU time : %f\n", ((double)(end - begin)) / CLOCKS_PER_SEC);
    } else if (sample_average) {
        double price, std_dev;
        begin = clock();
        mc.price(price, std_dev);
        end = clock();
        printf("mc price : %f\n", price);
        printf("std_dev : %f\n", std_dev);
        printf("CPU time : %f\n", ((double)(end - begin)) / CLOCKS_PER_SEC);
        printf("Score: %f\n", ((double)(end - begin)) * std_dev * std_dev);
        printf("\n");

        if (seed > 0) pnl_rng_sseed(mc.rng, seed);

        if (delta) {
            begin = clock();
            mc.price_delta_SAA(drift, price, std_dev, dprice, dpricestd_dev, verbose);

            end = clock();
            printf("price with sample averaging : %f\n", price);
            printf("std_dev with sample averaging : %f\n", std_dev);
            cout << "delta with sample averaging : ";
            pnl_vect_print_asrow(dprice);
            cout << "std_dev of delta with sample averaging : ";
            pnl_vect_print_asrow(dpricestd_dev);
            cout << "value of the drift : ";
            pnl_vect_print_asrow(drift);
            printf("CPU time : %f\n", ((double)(end - begin)) / CLOCKS_PER_SEC);
            printf("Score: %f\n", ((double)(end - begin)) * std_dev * std_dev);
        } else {
            PnlVect *mu = pnl_vect_new();
            begin = clock();
            mc.price_SAA(drift, price, std_dev, verbose);

            end = clock();
            printf("price with sample averaging : %f\n", price);
            printf("std_dev with sample averaging : %f\n", std_dev);
            cout << "value of the drift : ";
            pnl_vect_print_asrow(drift);

            pnl_vect_free(&mu);
            printf("CPU time : %f\n", ((double)(end - begin)) / CLOCKS_PER_SEC);
            printf("Score: %f\n", ((double)(end - begin)) * std_dev * std_dev);
        }
    }

    pnl_vect_free(&drift);
    pnl_vect_free(&dprice);
    pnl_vect_free(&dpricestd_dev);
}

/**
 * Run the right MonteCarlo routine
 *
 * @param rng PnlRng
 * @param map an instance of IParser
 * @param seed an integer to reset the ssed of the random number generator
 * @param verbose a boolean to activate message printing
 */
void testPricer(PnlRng *rng, const IParser &map, int seed, bool verbose)
{
    PnlVect *drift, *dpricestd_dev;
    double std_dev;
    clock_t begin, end;
    int nb_traj = 10000;

    drift = pnl_vect_new();
    dpricestd_dev = pnl_vect_new();

    // Case t = 0;
    pnl_rng_sseed(rng, seed);
    MonteCarlo *mc_time0 = new MonteCarlo(rng, map);
    double price_time0, price_time0_bis;
    PnlVect *dprice_time0 = pnl_vect_new();
    mc_time0->price_delta(price_time0, std_dev, dprice_time0, dpricestd_dev);
    if (verbose) {
        printf("mc price: %f\n", price_time0);
        printf("price std_dev: %f\n", std_dev);
        printf("mc delta: ");
        pnl_vect_print_asrow(dprice_time0);
        printf("std_dev delta: ");
        pnl_vect_print_asrow(dpricestd_dev);
    }
    mc_time0->price(price_time0_bis, std_dev);
    std::cout << "Compare prices from Price and PriceDelta: ";
    if (cmp_real_abs(price_time0, price_time0_bis, 2 * 1.96 * std_dev))
        std::cout << "OK.\n";
    else
        std::cout << "FAIL.\n";

    if (verbose) {
        printf("mc price: %f\n", price_time0_bis);
        printf("price std_dev: %f\n", std_dev);
    }

    // Case any time t > 0
    double price_timet;
    PnlVect *dprice_timet = pnl_vect_new();
    double t = mc_time0->mod->m_maturity / 2 + mc_time0->mod->m_dt * 0.1;
    double T;
    map.extract("maturity", T);
    T += t;
    IParser map_timet(map);
    map_timet.set("maturity", T);
    // Reset the generator to use the samples as above.
    pnl_rng_sseed(rng, seed);
    MonteCarlo *mc_timet = new MonteCarlo(rng, map_timet);
    PnlMat *past = pnl_mat_create(2, mc_timet->mod->m_modelSize);
    pnl_mat_set_row(past, mc_time0->mod->m_init, 1);
    mc_timet->price_delta(past, t, price_timet, std_dev, dprice_timet, dpricestd_dev);
    if (verbose) {
        printf("mc price at time t: %f\n", price_timet);
        printf("price std_dev at time t: %f\n", std_dev);
        printf("mc delta at time t: ");
        pnl_vect_print_asrow(dprice_timet);
        printf("std_dev delta at time t: ");
        pnl_vect_print_asrow(dpricestd_dev);
    }

    mc_timet->price(past, t, price_timet, std_dev);
    if (verbose) {
        printf("mc price at time t: %f\n", price_timet);
        printf("price std_dev at time t: %f\n", std_dev);
    }

    std::cout << "Compare Prices at time 0 and t: ";
    if (cmp_real_abs(price_time0_bis, price_timet, 1E-10))
        std::cout << "OK.\n";
    else
        std::cout << "FAIL.\n";

    std::cout << "Compare Deltas at time 0 and t: ";
    if (cmp_vect_abs(dprice_time0, dprice_timet, 1E-10))
        std::cout << "OK.\n";
    else
        std::cout << "FAIL.\n";

    delete mc_time0;
    delete mc_timet;

    pnl_vect_free(&drift);
    pnl_vect_free(&dprice_time0);
    pnl_vect_free(&dprice_timet);
    pnl_vect_free(&dpricestd_dev);
}

void hedgeWrapper(const char *marketFile, const IParser &map, PnlRng *rng)
{
    PnlMat *market = pnl_mat_new();
    MonteCarlo mc(rng, map);
    if (marketFile == NULL)
        mc.mod->simul_market(rng, market);
    else
        mc.mod->simul_market(mc.mod->m_modelSize, marketFile, market);

    clock_t begin, end;
    begin = clock();
    double pl = mc.hedge(market);
    end = clock();
    std::cout << "Hedging error: " << pl << std::endl;
    printf("CPU time : %f\n", ((double)(end - begin)) / CLOCKS_PER_SEC);
    pnl_mat_free(&market);
}

void pnlSampleWrapper(string pnlFile, int nPnlSamples, const IParser &map, PnlRng *rng)
{
    PnlMat *market = pnl_mat_new();
    MonteCarlo mc(rng, map);
    ofstream out(pnlFile);
    for (int i = 0; i < nPnlSamples; i++) {
        mc.mod->simul_market(rng, market);
        double pl = mc.hedge(market, false);
        out << pl << endl;
    }
    pnl_mat_free(&market);
}

int main(int argc, char **argv)
{
    bool price = false, delta = false, test = false, hedge = false;
    bool sample_average = false;
    bool verbose = false;
    int seed = 0, pnl = 0;
    string infile, market, pnlFile;

    cxxopts::Options options("mc-pricer", "Hedging tool");

    options.add_options()("help", "Print help message")
    ("test", "Run tests.", cxxopts::value<bool>())
    ("price-only", "Run a crude Monte Carlo", cxxopts::value<bool>())
    ("delta", "Compute the delta.", cxxopts::value<bool>())
    ("hedge", "Compute the hedging portfolio and return the P&L.", cxxopts::value<bool>())
    ("pnl", "Compute the PNL on n generated market paths.", cxxopts::value<int>()->default_value("0"))
    ("seed", "Fix the seed for the random generator. If none is given, use time(NULL).", cxxopts::value<int>()->default_value("0"))
    ("sample-average", "Use SAA to compute the optimal IS.", cxxopts::value<bool>())
    ("verbose", "Be more verbose.", cxxopts::value<bool>())
    ("infile", "Path to the data file", cxxopts::value<std::string>())
    ("market", "Path to the market file", cxxopts::value<std::string>())
    ("pnlfile", "Path to the PNL file", cxxopts::value<std::string>());

    auto vm = options.parse(argc, argv);
    if (vm.count("help") || !vm.count("infile")) {
        std::cout << options.help();
        return 1;
    }

    test = vm["test"].as<bool>();
    price = vm["price-only"].as<bool>();
    delta = vm["delta"].as<bool>();
    hedge = vm["hedge"].as<bool>();
    pnl = vm["pnl"].as<int>();
    verbose = vm["verbose"].as<bool>();
    sample_average = vm["sample-average"].as<bool>();
    infile = vm["infile"].as<std::string>();
    if (vm.count("market")) {
        market = vm["market"].as<std::string>();
    }
    if (vm.count("pnlfile")) {
        pnlFile = vm["pnlfile"].as<std::string>();
    }
    seed = vm["seed"].as<int>();

    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    if (seed == 0) seed = time(NULL);
    pnl_rng_sseed(rng, seed);

    // model and option initialisation
    IParser map = IParser(infile.c_str());

    if (test) {
        testPricer(rng, map, seed, verbose);
    } else if (hedge) {
        if (vm.count("market"))
            hedgeWrapper(market.c_str(), map, rng);
        else
            hedgeWrapper(NULL, map, rng);
    } else if (pnl) {
        pnlSampleWrapper(pnlFile, pnl, map, rng);
    } else if (price) {
        MonteCarloWrapper(rng, map, seed, price, delta, sample_average, verbose);
    }

    pnl_rng_free(&rng);
    exit(0);
}
