import numpy as np 

from BlackScholesModel import BlackScholesModel
from AsianOption import AsianOption
from typing import Callable

class MonteCarlo :


    def __init__(self , model : BlackScholesModel , option : AsianOption):
        self.model = model 
        self.option = option

    
    def price(self , M ) :
        """
        le prix à t = 0 
        """

        price = 0 

        for _ in range(M) :
            S = self.model.bsAsset()
            price += self.option.payoff(S)
        
        price *= np.exp(-self.model.r*self.option.T)
        price /= M

        return price 


    def price_mc(self , 
                M :int, 
                N : int  , 
                t : int , 
                alpha : float , 
                V_0 : float , 
                P : int ):
        """
        
        """
        price: float = 0.0 

        k : int = t*P / self.option.T
        
        for _ in range(M) :
            x : float = 0.0
            Y = self.model.simulation_incr_brownien(int(k))
            for _ in range(N) :
                X = self.model.simulation_incr_brownien(int(P - k));
                incr = np.concatenate((Y,X));
                S = self.model.bsAssetIncr(incr); 
                x += np.exp(-self.model.r*(self.model.T - t))*self.option.payoff(S) - alpha*V_0
            x /= N 
            price += np.exp(-self.model.r*t)*max(x , 0.0)
        
        price /= M

        return price 



