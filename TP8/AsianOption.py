import numpy as np


class AsianOption :

    def __init__(self , strike , T , N ):
        self.strike = strike
        self.T = T 
        self.N = N 

    def approximate_A(self  , S  ) :
        P = len(S) - 1
        res = (self.T/P)*(sum(S) - 0.5*(S[0] + S[-1]))
        return res 
    
    
    def payoff(self  , S ):
        
        return max((1/self.T)*self.approximate_A(S) - self.strike , 0 )



