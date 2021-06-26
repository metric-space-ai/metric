#ifndef _UPS_CALC_REWEIGHTING_HPP
#define _UPS_CALC_REWEIGHTING_HPP


#include "3rdparty/blaze/Blaze.h"

//#include <iostream>  // TODO remove



template <typename T>
std::vector<std::vector<blaze::DynamicVector<T>>>
calcReweighting(
        const std::vector<blaze::DynamicVector<T>> & rho,
        const blaze::DynamicMatrix<float> & sh,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const T lambda = 1
        )
{
    //std::cout << "calcReweighting started\n";

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    //size_t npix = flat_imgs[0][0].size();

    //auto rho_avg = blaze::DynamicVector<T>(rho[0].size(), 0);
    //for (size_t i = 0; i<rho.size(); ++i) {
    //    rho_avg += rho[i];
    //}
    //rho_avg /= rho.size();

    blaze::DynamicMatrix<T, blaze::columnMajor> s_mat (s[0][0].size(), nimages*nchannels);
    for (size_t i = 0; i < nimages*nchannels; ++i) {
        //blaze::column(s_mat, i) = s[i%nimages][i/nimages];
        blaze::column(s_mat, i) = s[i/nchannels][i%nchannels];
    }

    //std::cout << "s_mat:\n" << s_mat << "\n"; // TODO remove

    auto rk_mat = sh * s_mat;

    //std::cout << "rk_mat:\n" << rk_mat << "\n"; // TODO remove

    std::vector<std::vector<blaze::DynamicVector<T>>> weights = {};
    size_t w_idx = 0;
    for (size_t i = 0; i < nimages; ++i) {
        std::vector<blaze::DynamicVector<T>> rk_ch = {};
        for (size_t c = 0; c<nchannels; ++c) {
            blaze::DynamicVector<T> v = blaze::column(rk_mat, w_idx);
            v = v * rho[c];  // elementwize
            v = v - flat_imgs[i][c];
            v = blaze::pow(v, 2) / (lambda*lambda) + 1;
            v = 1 / v;
            rk_ch.push_back(v);
            ++w_idx;
        }
        weights.push_back(rk_ch);
    }

    //std::cout << "rk:\n" << rk << "\n"; // TODO remove

    return weights;
}



#endif // _UPS_CALC_REWEIGHTING_HPP
