from MonteCarlo import MonteCarlo
from AsianOption import AsianOption
from BlackScholesModel import BlackScholesModel

r = 0.03 
S0 = 100
K = 110
T = 2
sigma = 0.3 
t = 1
alpha = 0.8

P = 20
M = 1000
N = 1000

model : BlackScholesModel = BlackScholesModel(r , sigma  , P , T , S0)

option : AsianOption = AsianOption(K , T , P)

mc : MonteCarlo = MonteCarlo(model , option )

V_0 = mc.price(M)
print(V_0)
price = mc.price_mc(M , N , t , alpha , V_0 , P )

print(f"le prix d'une option d'achat de maturité t = 1 : {price}")