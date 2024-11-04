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
#include "HedgingResults.hpp"

using namespace std;

int main(int argc, char **argv)
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    unsigned long seed = (unsigned long)(time(NULL));
    pnl_rng_sseed(rng, seed);

    if (argc != 3) {
        cout << "Usage: ./generate-pnl-tojson market.txt file.json" << endl;
        exit(0);
    }

    char *infile = argv[2];
    char *marketFile = argv[1];
    IParser map = IParser(infile);
    PnlMat *market = pnl_mat_new();
    MonteCarlo mc(rng, map);
    double price, std_dev, pl, cpuTime;

    mc.mod->simul_market(mc.mod->m_modelSize, marketFile, market);
    clock_t begin, end;
    begin = clock();
    mc.price(price, std_dev);
    pl = mc.hedge(market, false);
    end = clock();
    cpuTime = double(end - begin) / CLOCKS_PER_SEC;

    HedgingResults res = HedgingResults(price, std_dev, pl, cpuTime);
    std::cout << res << std::endl;

    pnl_mat_free(&market);
    pnl_rng_free(&rng);
    exit(0);
}
