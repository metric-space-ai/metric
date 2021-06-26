#ifndef _UPS_INIT_HPP
#define _UPS_INIT_HPP


#include "modules/utils/solver/ups/helpers/indexing.hpp"
#include "modules/utils/solver/ups/helpers/console_output.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
#include <vector>
//#include <iostream>  // TODO remove


template <typename T>
std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>>
initRho(const std::vector<std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>>> & I) {

    // rho as median of I over images for each channel
    std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> rho = {};
    for (size_t ch_idx=0; ch_idx<I[0].size(); ++ch_idx) {
        auto rho_ch = blaze::DynamicMatrix<T, blaze::columnMajor>(I[0][0].rows(), I[0][0].columns(), 0);
        rho.push_back(rho_ch);
    }
    for (size_t i = 0; i<I[0][0].rows(); ++i) {
        for (size_t j = 0; j<I[0][0].columns(); ++j) {
            for (size_t ch_idx=0; ch_idx<I[0].size(); ++ch_idx) {
                std::vector<T> p = {};
                for (size_t img_idx=0; img_idx<I.size(); ++img_idx) {
                    p.push_back(I[img_idx][ch_idx](i, j));
                }
                std::nth_element(p.begin(), p.begin() + p.size()/2, p.end());
                T median = p[p.size()/2 - 1];
                if (p.size() % 2 == 0) {
                    median = (median + p[p.size()/2]) / 2;
                }
                rho[ch_idx](i, j) = median;
            }
        }
    }
    return rho;
}



template <typename T>
std::vector<std::vector<blaze::DynamicVector<T>>>
initS(const size_t nchannels, const size_t nimages, const blaze::DynamicVector<T> v = {0, 0, -1, 0.2}) {

    std::vector<std::vector<blaze::DynamicVector<T>>> s = {};
    for (size_t i = 0; i < nimages; ++i) {
        std::vector<blaze::DynamicVector<T>> s_ch = {};
        for (size_t c = 0; c<nchannels; ++c) {
            s_ch.push_back(v);
        }
        s.push_back(s_ch);
    }
    return s;
}



template <typename T>
std::tuple<std::vector<std::vector<blaze::DynamicVector<T>>>, std::vector<blaze::DynamicVector<T>>>
VariablesInitialization(
        const std::vector<std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>>> & I,
        //const blaze::CompressedMatrix<bool, blaze::columnMajor> & M,
        const blaze::DynamicMatrix<bool, blaze::columnMajor> & M,
        const std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> & rho//,
        //std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> s,
        //blaze::DynamicVector<T> z
        )
{
    size_t nimages = size(I);
    size_t nchannels = size(I[0]);  // we assume each image has equal number of channeks and resolution
    size_t nrows = I[0][0].rows();
    size_t ncols = I[0][0].columns();

    auto idc = indicesCwStd(M);

    //std::cout << "idc:\n" << idc << "\n\n";  // TODO remove

    // I
    std::vector<std::vector<blaze::DynamicVector<T>>> images;
    for (size_t i = 0; i < nimages; ++i) {
        std::vector<blaze::DynamicVector<T>> I_out = {};
        for (size_t c = 0; c<nchannels; ++c) {
            blaze::DynamicVector<T> flat_img = flattenToCol(I[i][c]);
            I_out.push_back(blaze::elements(flat_img, idc));  // apply mask and save
        }
        images.push_back(I_out);
    }

    // rho
    std::vector<blaze::DynamicVector<T>> rho_out = {};
    for (size_t c = 0; c<nchannels; ++c) {
        blaze::DynamicVector<T> flat_rho = flattenToCol(rho[c]);
        rho_out.push_back(blaze::elements(flat_rho, idc));  // apply mask and save
    }

    // z
    //blaze::DynamicVector<T> z_out = blaze::elements(z, idc);

    return(std::make_tuple(images, rho_out));

}



#endif // _UPS_INIT_HPP
