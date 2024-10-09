#include <iostream>
#include "pnl/pnl_vector.h"
#include "pnl/pnl_random.h"
#include <cmath>

//  Constante globale
double S_0 = 100.0;
double K = 1.2;
double r = 0.02;
double sigma = 0.25;
double T = 2.0;
int N = 24;
int M = 50000;

double price(PnlRng *rng , double& demi_largeur);
double price_anti(PnlRng *rng , double& demi_largeur_IC );
double price_controle(PnlRng* rng , double& demi_largeur);

int main()
{
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, time(NULL));

    double demi_mc = 0.0;
    double price_0_mc = price(rng , demi_mc);
    std::cout << "price_0_mc = " << price_0_mc << std::endl;
    std::cout << "demi_IC = " << demi_mc << std::endl;
    std::cout << "=============================" << std::endl;



    double demi_IC = 0.0;
    double price_0_anti = price_anti(rng , demi_IC);
    std::cout << "price_0_anti = " << price_0_anti << std::endl;
    std::cout << "demi_IC =  = " << demi_IC << std::endl;
    std::cout << "============================="  << std::endl;


    double demi_controle = 0.0;
    double price_0_controle = price_controle(rng , demi_controle);
    std::cout << "price_0_controle = " << price_0_controle << std::endl;
    std::cout << "demi_IC =  = " << demi_controle << std::endl;

    

    pnl_rng_free(&rng);
    return 0;
}

void simulation_s_t(PnlVect *res, PnlRng *rng)
{

    double dt = T / (double)N;
    pnl_vect_set(res, 0, S_0);
    for (int i = 1; i <= N; i++)
    {
        double s_t = pnl_vect_get(res, i - 1) * std::exp((r - sigma*sigma/ 2) * dt + sigma * std::sqrt(dt) * pnl_rng_normal(rng));
        pnl_vect_set(res, i, s_t);
    }
}

void simulation_shifft(PnlVect *res , PnlVect* res_shifft, PnlRng *rng)
{

    double dt = T / (double)N;
    pnl_vect_set(res, 0, S_0);
    pnl_vect_set(res_shifft, 0, S_0);

    for (int i = 1; i <= N; i++)
    {   
        double G_i = pnl_rng_normal(rng);
        double s_t = pnl_vect_get(res, i - 1) * std::exp((r - sigma*sigma/ 2) * dt + sigma * std::sqrt(dt)  * G_i);
        double s_t_shifft = pnl_vect_get(res_shifft, i - 1) * std::exp((r - sigma*sigma/ 2) * dt + sigma * std::sqrt(dt)  * G_i *(-1));
        pnl_vect_set(res, i, s_t);
        pnl_vect_set(res_shifft, i, s_t_shifft);



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
    double a = max_ - K*S_T;
    double res = a > 0 ? a : 0;
 
    return res;
}

double price(PnlRng *rng , double& demi_largeur)
{
    double price_0 = 0.0;
    demi_largeur = 0.0;

    PnlVect *sim = pnl_vect_create(N + 1);
    for (int i = 0; i < M; i++)
    {
        simulation_s_t(sim, rng);
        double p = payoff(sim);
        price_0 += p;
        demi_largeur+= p*p;
    }

    price_0 = std::exp(-r * T)*price_0 * (1.0 / (double)M);

    demi_largeur = exp(-2*r*T)*demi_largeur/(double(M)) - price_0*price_0;
    demi_largeur = sqrt(demi_largeur)*1.96/(sqrt(M));

    
    pnl_vect_free(&sim);
    return price_0;
}


double price_anti(PnlRng *rng , double& demi_largeur_IC )
{
    double price_0 = 0.0;
    int n = M / 2;

    demi_largeur_IC = 0.0 ; 

    PnlVect *sim = pnl_vect_create(N + 1);
    PnlVect *sim_shifft = pnl_vect_create(N + 1);

    for (int i = 0; i < n; i++)
    {
        simulation_shifft(sim, sim_shifft , rng);
        double p = payoff(sim) + payoff(sim_shifft);
        price_0 += p;
        demi_largeur_IC += p*p ;
    }

    price_0 = std::exp(-r * T)*price_0 * (1.0 / (double)M);

    demi_largeur_IC = (1.0/(2.0*(double)M))*demi_largeur_IC*std::exp(-2*r * T) - price_0*price_0;
    demi_largeur_IC = sqrt(demi_largeur_IC)*1.96/(sqrt(n));
    



    pnl_vect_free(&sim);
    pnl_vect_free(&sim_shifft);
    
    return price_0;
}



double price_controle(PnlRng* rng , double& demi_largeur) {

    double price_0 = 0.0;
    demi_largeur = 0.0;

    PnlVect *sim = pnl_vect_create(N + 1);

    PnlVect* p_ = pnl_vect_create(M);
    PnlVect* y_  = pnl_vect_create(M);


    double lambda = 0.0 ;
    double denom = 0.0 ; 

    for(int i = 0 ; i< M ; i++) {
        simulation_s_t(sim , rng);
        double S_T = pnl_vect_get(sim , sim->size - 1);

        double Y_i = -1*(S_T - S_0*exp(r*T));
        double p = exp(-r*T)*payoff(sim);

        pnl_vect_set(y_ , i , Y_i);
        pnl_vect_set(p_ , i , p);

        lambda += p*Y_i ;
        denom += Y_i*Y_i;
    }

    lambda = lambda / denom;
    std::cout << lambda << std::endl; // 0.25 

    for(int i = 0 ; i< M ; i++) {
        double a = pnl_vect_get(p_ , i) - lambda*pnl_vect_get(y_ , i);
        price_0+=a;
        demi_largeur+=a*a;
    }

    price_0 = price_0 /(double)M;
    demi_largeur = (demi_largeur / (double)M) - price_0*price_0;
    demi_largeur = sqrt(demi_largeur)*1.96/sqrt(M);

    return price_0;

}