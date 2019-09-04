/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_PCFA_HPP
#define _METRIC_MAPPING_PCFA_HPP

#include "../../3rdparty/blaze/Blaze.h"

namespace metric {

/**
 * @brief 
 * 
 * @param In 
 * @param n_components
 * @param averages - outputs average curve
 * @return 
 */
template <class BlazeMatrix>
blaze::DynamicMatrix<double> PCA(
        const BlazeMatrix & In,
        int n_components,
        blaze::DynamicVector<double> & averages
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
   */
    PCFA();

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
    blaze::DynamicMatrix<double> encode(const blaze::DynamicMatrix<double>& Slices);

    /**
   * @brief 
   * 
   * @param Codes 
   * @parem unshift - flag for adding average curve to each decoded one
   * @return blaze::DynamicMatrix<double> 
   */
    blaze::DynamicMatrix<double> decode(const blaze::DynamicMatrix<double>& Codes, bool unshift=true);

private:
    blaze::DynamicMatrix<double> W_decode;
    blaze::DynamicMatrix<double> W_encode;
    blaze::DynamicVector<double> averages;
    bool visualize = false;
    std::default_random_engine rgen;
};

}  // namespace metric

#endif  // _METRIC_MAPPING_PCFA_HPP

#include "PCFA.cpp"
