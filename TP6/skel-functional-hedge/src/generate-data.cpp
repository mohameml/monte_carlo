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

using namespace std;

int main(int argc, char **argv)
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    unsigned long seed = (unsigned long)(time(NULL));
    pnl_rng_sseed(rng, seed);

    if (argc != 2) {
        cout << "Usage: ./generate-data file.dat" << endl;
        exit(0);
    }

    char *infile = argv[1];
    IParser map = IParser(infile);

    PnlVect *dprice = pnl_vect_new();
    PnlVect *dpricestd_dev = pnl_vect_new();

    MonteCarlo mc(rng, map);

    double price, std_dev;
    clock_t begin, end;
    begin = clock();
    mc.price_delta(price, std_dev, dprice, dpricestd_dev);
    end = clock();

    printf("# MC price: %f\n", price);
    printf("# price std_dev: %f\n", std_dev);
    printf("# MC delta: ");
    pnl_vect_print_asrow(dprice);
    printf("# delta std_dev: ");
    pnl_vect_print_asrow(dpricestd_dev);
    printf("# CPU time : %f\n", ((double)(end - begin)) / CLOCKS_PER_SEC);

    pnl_vect_free(&dprice);
    pnl_vect_free(&dpricestd_dev);
    pnl_rng_free(&rng);
    exit(0);
}
