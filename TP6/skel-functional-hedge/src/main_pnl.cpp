#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <mpi.h>

#include "Model.hpp"
#include "Option.hpp"
#include "jlparser/parser.hpp"
#include "MonteCarlo.hpp"
#include "compare.hpp"
#include "pnl/pnl_mpi.h"

#include "cxxopts.hpp"
#define PNL_TAG 0

/**
 * @brief Compute the PNL on @p nPnlSamples generated market paths.
 *
 * @param[out] pnlSamples contains the PNL on each path on output.
 * @param nPnlSamples number of pnl samples to compute.
 * @param map an IParser object.
 * @param rng a PnlRng object.
 */
void pnlSampleWrapper(PnlVect *pnlSamples, int nPnlSamples, const IParser &map, PnlRng *rng)
{
    PnlMat *market = pnl_mat_new();
    pnl_vect_resize(pnlSamples, nPnlSamples);
    MonteCarlo mc(rng, map);
    for (int i = 0; i < nPnlSamples; i++) {
        mc.mod->simul_market(rng, market);
        double pl = mc.hedge(market, false);
        LET(pnlSamples, i) = pl;
    }
    pnl_mat_free(&market);
}

int main(int argc, char **argv)
{
    int mpi_rank, mpi_size;
    bool verbose = false;
    int seed = 0, nPaths = 0;
    std::string infile, pnlFile;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    cxxopts::Options options("pnl", "Hedging tool");

    options.add_options()("help", "Print help message")
    ("n,n-paths", "Compute the PNL on n generated market paths.", cxxopts::value<int>()->default_value("1"))
    ("seed", "Fix the seed for the random generator. If none is given, use time(NULL).", cxxopts::value<int>()->default_value("0"))
    ("verbose", "Be more verbose.", cxxopts::value<bool>())
    ("infile", "Path to the data file", cxxopts::value<std::string>())
    ("pnlfile", "Path to the PNL file", cxxopts::value<std::string>());

    auto vm = options.parse(argc, argv);
    if (vm.count("help") || !vm.count("infile") || !vm.count("pnlfile")) {
        std::cout << options.help();
        return 1;
    }

    nPaths = vm["n-paths"].as<int>();
    verbose = vm["verbose"].as<bool>();
    pnlFile = vm["pnlfile"].as<std::string>();
    infile = vm["infile"].as<std::string>();
    seed = vm["seed"].as<int>();

    if (seed == 0) seed = time(NULL);
    PnlRng *rng = pnl_rng_dcmt_create_id(mpi_rank, 0);
    pnl_rng_sseed(rng, seed);

    // model and option initialisation
    IParser map = IParser(infile.c_str());
    PnlVect *pnlSamples = pnl_vect_new();
    int nPnlSamplesPerSlave = int(ceil(double(nPaths) / double(mpi_size)));
    int nPnlSamplesRoot = nPaths - nPnlSamplesPerSlave * (mpi_size - 1);
    if (mpi_rank == 0) {
        pnlSampleWrapper(pnlSamples, nPnlSamplesRoot, map, rng);
        MPI_Status status;
        FILE *f = fopen(pnlFile.c_str(), "w");
        pnl_vect_fprint(f, pnlSamples);
        pnl_vect_resize(pnlSamples, nPnlSamplesPerSlave);
        for (int rank = 1; rank < mpi_size; rank++) {
            pnl_object_mpi_recv((PnlObject *)pnlSamples, MPI_ANY_SOURCE, PNL_TAG, MPI_COMM_WORLD, &status);
            pnl_vect_fprint(f, pnlSamples);
        }
        fclose(f);
    } else {
        pnlSampleWrapper(pnlSamples, nPnlSamplesPerSlave, map, rng);
        pnl_object_mpi_send((PnlObject *)pnlSamples, 0, PNL_TAG, MPI_COMM_WORLD);
    }
    pnl_vect_free(&pnlSamples);
    pnl_rng_free(&rng);
    MPI_Finalize();
    exit(0);
}
