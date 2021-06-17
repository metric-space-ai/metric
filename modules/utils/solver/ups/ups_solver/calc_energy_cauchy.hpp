#ifndef _UPS_CALC_ENERGY_CAUCHY_HPP
#define _UPS_CALC_ENERGY_CAUCHY_HPP

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
#include <iostream>  // TODO remove


// I, rho, s, sh, theta, drho, dz,  u, params, options

template <typename T>
std::tuple<T, T, T>
calcEnergyCauchy(
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const std::vector<blaze::DynamicVector<T>> & rho,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const blaze::DynamicMatrix<T> & sh,
        const blaze::DynamicVector<T> & theta,
        const std::vector<blaze::DynamicVector<T>> & drho,
        const blaze::DynamicVector<T> & dz,
        const blaze::DynamicVector<T> & u,
        T lambda = 1.0,
        T huber = 0.1,
        T mu = 0.000002,
        T beta = 0.0005
        )
{
    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    size_t npix = flat_imgs[0][0].size();

    T energy = 0;
    for (size_t im = 0; im < s.size(); ++im) {
        for (size_t ch = 0; ch < s[0].size(); ++ch) {
            blaze::DynamicVector<T> tmp = (sh * s[im][ch])*rho[ch];
            energy += blaze::sum(blaze::pow(tmp - flat_imgs[im][ch], 2));
        }
    }
    energy = lambda/2*energy;
    T energy_no_smooth = energy;

    //std::cout << std::endl << "energy_no_smooth: " << energy << std::endl;  // TODO remove

    // if(options.regular==1)
    T huber_summand = 0;
    for (size_t ch = 0; ch < drho.size(); ++ch) {
        for (size_t el = 0; el < drho[ch].size(); el++) {
            if (drho[ch][el] >= huber) {
                huber_summand += blaze::abs(drho[ch][el]) - huber*0.5;
            } else {
                huber_summand += 0.5*blaze::pow(drho[ch][el], 2) / huber;
            }
        }
    }
    energy += mu*huber_summand;

    //std::cout << std::endl << "energy: " << energy << std::endl;  // TODO remove

    T objective = energy + 0.5*beta * blaze::sum(blaze::pow(theta - dz + u, 2));

    //std::cout << std::endl << "objective: " << objective << std::endl;  // TODO remove

    return std::make_tuple(energy, objective, energy_no_smooth);

}



#endif // _UPS_CALC_ENERGY_CAUCHY_HPP
