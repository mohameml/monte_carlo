#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "pnl/pnl_mathtools.h"
#include "compare.hpp"

/**
 * Absolute comparison of two real numbers
 *
 * @param x real number
 * @param y real number
 * @param abserr real number defining the absolute error
 *
 * @return  true or false
 */
bool cmp_real_abs(double x, double y, double abserr)
{
    if ((isnan(x) && !isnan(y)) || (!isnan(x) && isnan(y))
        || (isinf(x) && !isinf(y)) || (!isinf(x) && isinf(y))) return false;
    if (isinf(x) && isinf(y)) {
        if (x * y > 0.)
            return false;
        else
            return true;
    }
    if (isnan(x) && isnan(y)) return true;
    return (fabs(x - y) > abserr ? false : true);
}

/**
 * Check if |x(i,j) - y(i,j)| < abserr
 *
 * @param X computed result (vector)
 * @param Y expected result (vector)
 * @param abserr absolute error
 *
 * @return false or true
 */
bool cmp_vect_abs(const PnlVect *X, const PnlVect *Y, double abserr)
{
    if (X->size != Y->size) {
        std::cout << "size mismatch in PnlVect comparison" << std::endl;
        return false;
    }

    int i;
    bool status;
    for (i = 0; i < X->size; i++) {
        const double x = GET(X, i);
        const double y = GET(Y, i);
        status = cmp_real_abs(x, y, abserr);
        if (!status) break;
    }
    if (!status) {
        printf(" expected %.18g observed %.18g\n", GET(Y, i), GET(X, i));
        return false;
    }
    return true;
}
