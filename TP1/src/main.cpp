#include <iostream>
#include "pnl/pnl_vector.h"
#include "pnl/pnl_random.h"
#include "math.h"
#include "pnl/pnl_specfun.h"
#include  <string>
#include <fstream>

void simulation_of_brownien(PnlVect *res, double T, int N, PnlRng *rng);
void test(double  a , double M ,  PnlRng* rng );
double estimation_taux_a(double T , int N , PnlRng* rng , int M , double a) ;
double calcul_taux_a_theorique(double a , double T);
void test_biais(const std::string& file_name, int nbs, PnlRng* rng);

int main()
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, time(NULL));

    double a = 2.0  ; 
    int M = 1000; 

    test(a , M , rng);

    test_biais("tests/data.txt" , 200 ,  rng);



    pnl_rng_free(&rng);
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


double estimation_taux_a(double T , int N , PnlRng* rng , int M , double a) {

    PnlVect* res = pnl_vect_create(M);

    double setp = T/(double)N;

    for(int i = 0 ; i <M ; i++) {
        PnlVect*  sim_brow = pnl_vect_create(N + 1);
        simulation_of_brownien(sim_brow , T , N , rng);

        for(int j = 0 ; j < N + 1 ; j++) {
            double w_t = pnl_vect_get(sim_brow , j);
            if(w_t >= a || j == N) {
                pnl_vect_set(res , i , j*setp);
                break;
            }
        }   
    }



    double taux_a = 0.0 ; 

    for(int i = 0 ; i< M ; i++) {
        taux_a+=pnl_vect_get(res , i);
    }

    taux_a = taux_a*(1/(double)M);


    pnl_vect_free(&res);
    return taux_a ; 


}

double calcul_taux_a_theorique(double a , double T) {

    double gamma_plus = pnl_sf_gamma_inc(1.0/2.0 , a*a/(2.0*T));
    double gamma_moins = pnl_sf_gamma_inc(-1.0/2.0 , a*a/(2.0*T));
    double inv_sqrt_PI = M_2_SQRTPI / 2;
    double res = T*(1 - gamma_plus*inv_sqrt_PI) + (a*a*inv_sqrt_PI / 2.0 )*gamma_moins;

    return res ; 

}


void test(double  a , double M ,  PnlRng* rng ) {

    double biais = 0.0 ; 

    for (int i = 1 ; i < 6  ; i++) {
        double T = i ;
        double N = 20*T ; 
        double taux_a_estimation = estimation_taux_a(T , N , rng , M , a);
        double taux_a_theorique = calcul_taux_a_theorique(a , T);

        std::cout << "taux_a_estimation = " << taux_a_estimation << std::endl ;
        std::cout << "taux_a_theorique = " << taux_a_theorique << std::endl ; 
        std::cout << "==============================================" << std::endl;

        biais+= taux_a_estimation - taux_a_theorique;

    }
    std::cout << "biais = " << biais /5 << std::endl ; 
}


void test_a_1(PnlRng* rng ) {

    double a = 1.0 ; 
    double T = 3.0 ;
    double M = 500000;

    for(int i = 10 ; i < 1000 ; i+=20) {

        double taux_a_estimation = estimation_taux_a(T , i , rng , M , a);
        double taux_a_theorique = calcul_taux_a_theorique(a , T);

        std::cout << "taux_a_estimation = " << taux_a_estimation << std::endl ;
        std::cout << "taux_a_theorique = " << taux_a_theorique << std::endl ; 
        std::cout << "==============================================" << std::endl;
    }

    
}




void test_biais(const std::string& file_name, int nbs, PnlRng* rng) {
    double T = 3.0;
    double a = 1.0;
    int M = 50000;

    std::ofstream file(file_name.c_str());

    if (!file.is_open()) {
        exit(1);
    }

    for (int i = 10; i < nbs; i++) {
        int N = i; 
        double taux_a_estimation = estimation_taux_a(T, N, rng, M, a);
        double taux_a_theorique = calcul_taux_a_theorique(a, T);

        file << std::log(N) << "," << std::log(taux_a_estimation)  << "\n";

    }

    file.close(); 
}