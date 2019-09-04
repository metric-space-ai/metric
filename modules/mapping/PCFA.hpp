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
blaze::DynamicMatrix<typename BlazeMatrix::ElementType> PCA(
        const BlazeMatrix & In,
        int n_components,
        blaze::DynamicVector<typename BlazeMatrix::ElementType> & averages
        );

/**
 * @class PCAnet
 * 
 *@brief simple linear encoder based on PCA
 * 
 */
template <typename V>
class PCFA {

public:
    using value_type = V;

    /**
   * @brief Construct a new PCAnet object
   * 
   * @param Slices 
   * @param n_features 
   */
    PCFA(const blaze::DynamicMatrix<value_type>& Slices, size_t n_features = 1);

    /**
   * @brief 
   * 
   * @param Slices 
   * @return
   */
    blaze::DynamicMatrix<value_type> encode(const blaze::DynamicMatrix<value_type>& Slices);

    /**
   * @brief 
   * 
   * @param Codes 
   * @parem unshift - flag for adding average curve to each decoded one
   * @return blaze::DynamicMatrix<value_type>
   */
    blaze::DynamicMatrix<value_type> decode(const blaze::DynamicMatrix<value_type>& Codes, bool unshift=true);

private:
    blaze::DynamicMatrix<value_type> W_decode;
    blaze::DynamicMatrix<value_type> W_encode;
    blaze::DynamicVector<value_type> averages;
    std::default_random_engine rgen;
};

}  // namespace metric

#endif  // _METRIC_MAPPING_PCFA_HPP

#include "PCFA.cpp"
