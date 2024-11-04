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
#include "PricingResults.hpp"

using namespace std;

int main(int argc, char **argv)
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    unsigned long seed = (unsigned long)(time(NULL));
    pnl_rng_sseed(rng, seed);

    if (argc != 2) {
        cout << "Usage: ./generate-price0-tojson file.json" << endl;
        exit(0);
    }

    char *infile = argv[1];
    IParser map = IParser(infile);

    PnlVect *dprice = pnl_vect_new();
    PnlVect *dpricestd_dev = pnl_vect_new();

    MonteCarlo mc(rng, map);

    double price, std_dev, cpuTime;
    clock_t begin, end;
    begin = clock();
    mc.price_delta(price, std_dev, dprice, dpricestd_dev);
    end = clock();
    cpuTime = double(end - begin) / CLOCKS_PER_SEC;

    PricingResults res = PricingResults(price, std_dev, dprice, dpricestd_dev, cpuTime);
    std::cout << res << std::endl;

    pnl_vect_free(&dprice);
    pnl_vect_free(&dpricestd_dev);
    pnl_rng_free(&rng);
    exit(0);
}
