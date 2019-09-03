/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_PCANET_HPP
#define _METRIC_MAPPING_PCANET_HPP

#include "../../3rdparty/blaze/Blaze.h"

namespace metric {

/**
 * @brief 
 * 
 * @param In 
 * @param n_components 
 * @param visualize 
 * @return 
 */
template <class BlazeMatrix>
blaze::DynamicMatrix<double> PCA(
        const BlazeMatrix & In,
        int n_components,
        blaze::DynamicVector<double> & averages,
        bool visualize = false
        );

/**
 * @class PCAnet
 * 
 *@brief simple linear encoder based on PCA
 * 
 */
class PCFA {

public:
    /**
   * @brief Construct a new PCAnet object
   * 
   * @param visualize_ 
   */
    PCFA(bool visualize_ = false);

    /**
   * @brief 
   * 
   * @param Slices 
   * @param n_features 
   */
    void train(const blaze::DynamicMatrix<double>& Slices, size_t n_features = 1);

    /**
   * @brief 
   * 
   * @param Slices 
   * @return
   */
    blaze::DynamicMatrix<double> compress(const blaze::DynamicMatrix<double>& Slices);

    /**
   * @brief 
   * 
   * @param Codes 
   * @return blaze::DynamicMatrix<double> 
   */
    blaze::DynamicMatrix<double> decompress(const blaze::DynamicMatrix<double>& Codes);

private:
    blaze::DynamicMatrix<double> W_decode;
    blaze::DynamicMatrix<double> W_encode;
    blaze::DynamicVector<double> averages;
    bool visualize = false;
    std::default_random_engine rgen;
};

}  // namespace metric
#include "PCAnet.cpp"

#endif  // _METRIC_MAPPING_DETAILS_ENCODER_HPP
