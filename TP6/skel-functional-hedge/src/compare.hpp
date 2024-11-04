#ifndef _COMPARE_HPP
#define _COMPARE_HPP

#include "pnl/pnl_vector.h"

/**
 * Absolute comparison of two real numbers
 *
 * @param x real number
 * @param y real number
 * @param abserr real number defining the absolute error
 *
 * @return  true or false
 */
bool cmp_real_abs(double x, double y, double abserr);
/**
 * Check if |x(i,j) - y(i,j)| < abserr
 *
 * @param X computed result (vector)
 * @param Y expected result (vector)
 * @param abserr absolute error
 *
 * @return false or true
 */
bool cmp_vect_abs(const PnlVect *X, const PnlVect *Y, double abserr);

#endif /* _COMPARE_HPP */
