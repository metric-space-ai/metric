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
            //: padWidth(kernelWidth - 1), padHeight(kernelHeight - 1) {
            //: Convolution2d<T, Channels>(imageWidth, imageHeight, kernelWidth, kernelHeight) {//}, this->padWidth(0), this->padHeight(0) {
        {
            this->padWidth = 0;
            this->padHeight = 0;

            metric::PadDirection pd = metric::PadDirection::POST;
            metric::PadType pt = metric::PadType::CIRCULAR;
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
        int sign = 1;
        for (size_t i = 0; i < scaling.size(); ++i) {
            scaling[i] = scaling[i]*coeff;
            wavelet[wavelet.size() - i - 1] = scaling[i] * sign;
            sign *= -1;
        }

        Container2d ll (order, order);
        Container2d lh (order, order);
        Container2d hl (order, order);
        Container2d hh (order, order);
        El ll_el, lh_el, hl_el, hh_el;
        for (size_t i = 0; i < scaling.size(); ++i) {
            for (size_t j = 0; j <= i; ++j) {
                ll_el = scaling[i] * scaling[j];
                ll(i, j) = ll_el;
                ll(j, i) = ll_el;
                lh_el = scaling[i] * wavelet[j]; // TODO check order!!
                lh(i, j) = lh_el;
                lh(j, i) = lh_el;
                hl_el = wavelet[i] * scaling[j];
                hl(i, j) = hl_el;
                hl(j, i) = hl_el;
                hh_el = wavelet[i] * wavelet[j];
                hh(i, j) = hh_el;
                hh(j, i) = hh_el;
            }
        }

        return std::make_tuple(ll, lh, hl, hh);

    }


}




#endif // WAVELET2D_HPP
