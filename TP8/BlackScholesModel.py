import numpy  as np 
from numpy.typing import NDArray

class BlackScholesModel : 
    """
    BlackScholes model 
    
    """


    def __init__(self , r , sigma ,N , T  , spot):
        self.r = r 
        self.sigma  = sigma
        self.N = N 
        self.T = T 
        self.spot = spot
        self.dt = T/N



    def simulation_incr_brownien(self , k) :
        """
        Simule un mouvement brownien sur l'intervalle [0, T].
        """
        G = np.random.randn(k)
        incr = np.sqrt(self.dt)*G 

        return incr



    def simulation_brownien(self) :
        """
        Simule un mouvement brownien sur l'intervalle [0, T].
        """
        G = np.random.randn(self.N)
        dt = self.T/self.N
        incr = np.sqrt(dt)*G 
        res = np.concatenate(([0.] , np.cumsum(incr)))
        return res 




    def bsAssetIncr(self , incr : NDArray[any] ) :
        """
        Simulation de trajectoires du prix d'un actif sous-jacent dans le modèle de Black-Scholes.
        """
        dt = self.T / len(incr)
        res = np.zeros(len(incr) + 1)
        res[0] = self.spot 
        for i in range(1, len(incr) + 1) :
            res[i] = res[i -1]*np.exp((self.r - self.sigma**2/2)*dt + self.sigma*incr[i - 1])

        return res ; 



    def bsAsset(self) :
        """
        Simulation de trajectoires du prix d'un actif sous-jacent dans le modèle de Black-Scholes.
        """
        t = np.linspace(0,self.T , self.N + 1)
        W = self.simulation_brownien()
        res = self.spot*np.exp((self.r - 0.5*(self.sigma**2))*t + self.sigma*W)
        return res






