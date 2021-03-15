/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_TRANSFORM_WAVELET2D_HPP
#define _METRIC_TRANSFORM_WAVELET2D_HPP


#include "../../modules/utils/image_processing/convolution.hpp"
#include "wavelet.hpp"
//#include "../../3rdparty/blaze/Blaze.h"

#include <cmath> // for only sqrt


namespace wavelet {

    template <typename T, size_t Channels>
    class Convolution2dCustom : public metric::Convolution2d<T, Channels> {

      public:
        Convolution2dCustom(
                size_t imageWidth,
                size_t imageHeight,
                size_t kernelWidth,
                size_t kernelHeight
                //const PadDirection pd = PadDirection::POST,
                //const PadType pt = PadType::CIRCULAR,
                //const size_t stride = 1
                )
        {
//            this->padWidth = kernelWidth - 1;
//            this->padHeight = kernelHeight - 1;
//            metric::Convolution2d<T, Channels>(imageWidth, imageHeight, kernelWidth, kernelHeight); // TODO remove


            this->padWidth = 0;
            this->padHeight = 0;

            metric::PadDirection pd = metric::PadDirection::POST;
            //metric::PadDirection pd = metric::PadDirection::BOTH;
            metric::PadType pt = metric::PadType::CIRCULAR;
            //metric::PadType pt = metric::PadType::REPLICATE;
            size_t stride = 2;

            this->padModel = std::make_shared<metric::PadModel<T>>(pd, pt, 0);

            //auto t1 = Clock::now();
            this->convLayer = std::make_shared<typename metric::Convolution2d<T, Channels>::ConvLayer2d>(imageWidth + this->padWidth, imageHeight + this->padHeight, 1, 1, kernelWidth, kernelHeight, stride);
            //auto t2 = Clock::now();
            //auto d = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

        }


    };


    template <typename Container2d>
    std::tuple<Container2d, Container2d, Container2d, Container2d>
    create2dKernels(int order) {

        using El = typename Container2d::ElementType; // now we support only Blaze matrices

        assert(order % 2 == 0);

        El coeff = 2/sqrt(2);

        std::vector<El> scaling (order);
        std::vector<El> wavelet (order);
        scaling = wavelet::dbwavf<std::vector<El>>(order/2, coeff);
        int sign = -1;  // because we start filling wavelet vwctorfrom the last point
        for (size_t i = 0; i < scaling.size(); ++i) {
            scaling[i] = scaling[i]*coeff;
            wavelet[wavelet.size() - i - 1] = scaling[i] * sign;
            sign *= -1;
        }

        Container2d ll (order, order);
        Container2d lh (order, order);
        Container2d hl (order, order);
        Container2d hh (order, order);
        El ll_el, hh_el;
        for (size_t i = 0; i < scaling.size(); ++i) {
            for (size_t j = 0; j <= i; ++j) {  // hh, ll are symmetric cases
                ll_el = scaling[i] * scaling[j];
                ll(i, j) = ll_el;
                ll(j, i) = ll_el;
                hh_el = wavelet[i] * wavelet[j];
                hh(i, j) = hh_el;
                hh(j, i) = hh_el;
            }
        }
        for (size_t i = 0; i < scaling.size(); ++i) {
            for (size_t j = 0; j < scaling.size(); ++j) {  // non-symmetric, loop through all values
                lh(i, j) = scaling[i] * wavelet[j];
                hl(i, j) = wavelet[i] * scaling[j];
            }
        }

        return std::make_tuple(ll, lh, hl, hh);
    }



    template <typename T>
    std::tuple<blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>>
    dwt2_conv2(blaze::DynamicMatrix<T> const & x, std::tuple<blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>> const & kernels) { // based on 2d convolution

        auto ll_k = std::get<0>(kernels);
        auto lh_k = std::get<1>(kernels);
        auto hl_k = std::get<2>(kernels);
        auto hh_k = std::get<3>(kernels);

        assert(ll_k.rows()==lh_k.rows());
        assert(ll_k.rows()==hl_k.rows());
        assert(ll_k.rows()==hh_k.rows());
        assert(ll_k.columns()==lh_k.columns());
        assert(ll_k.columns()==hl_k.columns());
        assert(ll_k.columns()==hh_k.columns());

        auto c2d = wavelet::Convolution2dCustom<double, 1>(x.rows(), x.columns(), ll_k.rows(), ll_k.columns());
        //auto c2d = metric::Convolution2d<double, 1>(x.rows(), x.columns(), ll_k.rows(), ll_k.columns());
        blaze::StaticVector<blaze::DynamicMatrix<double>, 1> vx {x};
        auto ll = c2d(vx, ll_k);
        auto lh = c2d(vx, lh_k);
        auto hl = c2d(vx, hl_k);
        auto hh = c2d(vx, hh_k);

        return std::make_tuple(ll[0], lh[0], hl[0], hh[0]);
    }
}




#endif // WAVELET2D_HPP
