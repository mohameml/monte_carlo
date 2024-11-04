#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdio>
#ifdef _USE_MPI_
#include <mpi.h>
#include "pnl/pnl_mpi.h"
#endif

#include "Model.hpp"
#include "Option.hpp"
#include "jlparser/parser.hpp"
#include "FunctionalHedger.hpp"
#include "compare.hpp"

#include "cxxopts.hpp"

using namespace std;

double hedgeWrapper(const char *marketFile, const IParser &map, const FunctionalHedger &functionalHedger, PnlRng *rng)
{
    PnlMat *market = pnl_mat_new();
    MonteCarlo mc(rng, map);
    if (marketFile == NULL)
        mc.mod->simul_market(rng, market);
    else
        mc.mod->simul_market(mc.mod->m_modelSize, marketFile, market);
    double price0, var0;
    mc.price(price0, var0);
    double PnL = functionalHedger.pnl(price0, market);
    pnl_mat_free(&market);
    return PnL;
}

void pnlSampleWrapper(string pnlFile, int nPnlSamples, const IParser &map, const FunctionalHedger &functionalHedger, PnlRng *rng, int rank = 0, int size = 1)
{
    int PNL_TAG = 1;
    PnlMat *market = pnl_mat_new();
    MonteCarlo mc(rng, map);
    double price0, stdDev0;
    mc.price(price0, stdDev0);
    if (rank == 0) {
        printf("mc price: %f\n", price0);
        printf("price std_dev: %f\n", stdDev0);
    }
    int nLocalPnlSamples = int(ceil(nPnlSamples / double(size)));
    if (rank == 0) {
        nLocalPnlSamples = nPnlSamples - (size - 1) * nLocalPnlSamples;
    }
    PnlVect *PnLArray = pnl_vect_create(nLocalPnlSamples);
    for (int i = 0; i < nLocalPnlSamples; i++) {
        mc.mod->simul_market(rng, market);
        LET(PnLArray, i) = functionalHedger.pnl(price0, market);
    }
    if (rank > 0) {
#ifdef _USE_MPI_
        pnl_object_mpi_send((PnlObject*) PnLArray, 0, PNL_TAG, MPI_COMM_WORLD);
#endif
    } else {
        FILE *out = fopen(pnlFile.c_str(), "w");
        pnl_vect_fprint(out, PnLArray);
#ifdef _USE_MPI_
        MPI_Status status;
        for (int i = 1; i < size; i++) {
            pnl_object_mpi_recv((PnlObject*) PnLArray, MPI_ANY_SOURCE, PNL_TAG, MPI_COMM_WORLD, &status);
            pnl_vect_fprint(out, PnLArray);
        }
#endif
        fclose(out);
    }
    pnl_mat_free(&market);
    pnl_vect_free(&PnLArray);
}

int main(int argc, char **argv)
{
    bool hedge = false, verbose = false;
    bool sample_average = false;
    int seed = 0, n_paths = 1;
    string infile, market, pnlFile;
#ifdef _USE_MPI_
    MPI_Init(&argc, &argv);
#endif

    cxxopts::Options options("functional-hedge", "Functional hedge tool");

    options.add_options()("help", "Print help message")
    ("n,n-paths", "Compute the PNL on n generated market paths.", cxxopts::value<int>()->default_value("1"))
    ("seed", "Fix the seed for the random generator. If none is given, use time(NULL).", cxxopts::value<int>()->default_value("0"))
    ("verbose", "Be more verbose.", cxxopts::value<bool>())
    ("i,infile", "Path to the data file", cxxopts::value<std::string>())
    ("market", "Path to the market file", cxxopts::value<std::string>())
    ("pnlfile", "Path to the PNL file", cxxopts::value<std::string>());

    auto vm = options.parse(argc, argv);
    if (vm.count("help") || !vm.count("infile")) {
        std::cout << options.help();
        return 1;
    }

    n_paths = vm["n-paths"].as<int>();
    verbose = vm["verbose"].as<bool>();
    infile = vm["infile"].as<std::string>();
    if (vm.count("market")) {
        market = vm["market"].as<std::string>();
        hedge = true;
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
    MonteCarlo mc(rng, map);
    FunctionalHedger functionalHedger(&mc, map);
    functionalHedger.compute();

    int rank = 0, size = 1;
#ifdef _USE_MPI_
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // Use an independent generator on a each process to distribute the P&L computations
    pnl_rng_free(&rng);
    rng = pnl_rng_dcmt_create_id(rank, 12345);
    pnl_rng_sseed(rng, seed);
#endif
    if (hedge && rank == 0) {
        hedgeWrapper(market.c_str(), map, functionalHedger, rng);
    }

    if (!hedge) {
        pnlSampleWrapper(pnlFile, n_paths, map, functionalHedger, rng, rank, size);
    }

    pnl_rng_free(&rng);

    #ifdef _USE_MPI_
    MPI_Finalize();
    #endif

    exit(0);
}
