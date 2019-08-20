/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_ESN_HPP
#define _METRIC_MAPPING_ESN_HPP

#include "../../3rdparty/blaze/Blaze.h"

namespace metric {

/**
 * @class ESN
 * 
 * @brief 
 */
class ESN {
public:
    /**
     * @brief Construct a new ESN object
     * 
     * @param w_size 
     * @param w_connections 
     * @param w_sr 
     * @param alpha_ 
     * @param washout_ 
     * @param beta_ 
     */
    ESN(size_t w_size = 500,  // number of elements in reservoir
        double w_connections = 10,  // number of interconnections (for each reservoir element)
        double w_sr = 0.6,  // desired spectral radius of the reservoir
        double alpha_ = 0.5,  // leak rate, number of slices excluded from output for washout
        size_t washout_ = 1,
        double beta_ = 0.5  // ridge solver metaparameter
    );

    /**
     * @brief 
     * 
     * @param Slices 
     * @param Target 
     */
    void train(const blaze::DynamicMatrix<double>& Slices, const blaze::DynamicMatrix<double>& Target);

    /**
     * @brief 
     * 
     * @param Slices 
     * @return 
     */
    blaze::DynamicMatrix<double> predict(const blaze::DynamicMatrix<double>& Slices);

private:
    blaze::DynamicMatrix<double> W_in;
    blaze::CompressedMatrix<double> W;
    blaze::DynamicMatrix<double> W_out = blaze::DynamicMatrix<double>(0, 0);
    bool trained = false;
    double alpha = 0.5;
    double beta = 0.5;
    size_t washout = 1;
    std::default_random_engine rgen;

    void create_W(size_t w_size, double w_connections, double w_sr);

};  // class ESN

}  // namespace metric

#include "ESN.cpp"

#endif  // _METRIC_MAPPING_DETAILS_ESN_HPP
