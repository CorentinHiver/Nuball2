#ifndef FITS_HPP
#define FITS_HPP

#include "libCo.hpp"

/**
 * @brief Fits a true peak shape for HPGe
 * @note Meant to be an interface with root TF1
 * @details
 * 
 * parameters 1-3 : sigmoid for the left step
 * parameters 4-6 : normal gaussian
 * parameters 7-9 : second gaussian for neutrons loss
 * 
 * @param xx: spectrum data
 * @param pp: 0 : null 1 : back const | 2 : back slope | 3 : back exp | 4 : amplitude | 5 : mean | 6 : resolution (2.35482*sigma) | 7 : Lambda | 8 : Rho | 9 : S
 * @return double 
 */
double DoubleTailedStepedGaussian(double *xx, double *pp)
{
    double f_tot = 0.;

    double const & Back_const = pp[1];
    double const & Back_slope = pp[2];
    double const & Back_Exp = pp[3];

    f_tot += (Back_const + (xx[0]) * Back_slope) * exp((xx[0]) * Back_Exp);

    double const & Ampli = pp[4];
    double const & Mean = pp[5];
    double const & Sigma = pp[6] * 1. / sqrt(8. * log(2.));
    double const & Lambda = pp[7];
    double const & Rho = pp[8];
    double const & S = pp[9];

    double const & U = (xx[0] - Mean) / Sigma;
    double const & f_g = Ampli * std::exp(-U * U * 0.5);
    double const & f_lambda = Ampli * std::exp(-0.5 * Lambda * (2. * U - Lambda));
    double const & f_rho = Ampli * std::exp(-0.5 * Rho * (2. * U - Rho));
    double const & f_S = Ampli * S * 1. / ((1 + std::exp(U)) * (1 + std::exp(U)));

    if (U < Lambda)
        f_tot += f_lambda;
    else if (U > Rho)
        f_tot += f_rho;
    else
        f_tot += f_g;

    f_tot += f_S;

    return f_tot;
}

#endif //FITS_HPP