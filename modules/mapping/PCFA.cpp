/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_MAPPING_PCFA_CPP
#define _METRIC_MAPPING_PCFA_CPP

#include "PCFA.hpp"


namespace metric {

template <class BlazeMatrix>
blaze::DynamicMatrix<typename BlazeMatrix::ElementType> PCA(const BlazeMatrix & In, int n_components, blaze::DynamicVector<typename BlazeMatrix::ElementType> & averages)
{
    auto Result = blaze::DynamicMatrix<typename BlazeMatrix::ElementType>(n_components, In.rows(), 0);

    averages = blaze::sum<blaze::rowwise>(In) / In.columns();
    auto CenteredInput = blaze::DynamicMatrix<typename BlazeMatrix::ElementType>(In.rows(), In.columns(), 0);
    for (size_t col = 0; col < In.columns(); col++)
        column(CenteredInput, col) = column(In, col) - averages;

    blaze::SymmetricMatrix<blaze::DynamicMatrix<typename BlazeMatrix::ElementType>> CovMat = blaze::evaluate(CenteredInput * trans(CenteredInput));

    blaze::DynamicVector<typename BlazeMatrix::ElementType, blaze::columnVector> w(In.rows());  // for eigenvalues
    blaze::DynamicMatrix<typename BlazeMatrix::ElementType, blaze::rowMajor> V(In.rows(), In.rows());  // for eigenvectors

    eigen(CovMat, w, V);

    // sort and select
    size_t lower_idx = 0;
    size_t upper_idx = w.size() - 1;
    //typename BlazeMatrix::ElementType spectral_radius;  // also we get spectral radius for normalization: we process the first eigenvalue specially
    //if ((-w[lower_idx] > w[upper_idx]))
    //    spectral_radius = w[lower_idx];
    //else
    //    spectral_radius = w[upper_idx];
    int count = 0;
    while (count < n_components && upper_idx > lower_idx) {
        if (-w[lower_idx] > w[upper_idx]) {
            blaze::row(Result, count) = blaze::row(V, lower_idx); // / spectral_radius;  // add eigenpair
            lower_idx++;
        } else {
            blaze::row(Result, count) = blaze::row(V, upper_idx); // / spectral_radius;  // add eigenpair
            upper_idx--;
        }
        count++;
    }

    return Result;
}

// simple linear encoder based on PCA
template <typename V>
PCFA<V>::PCFA(const blaze::DynamicMatrix<value_type>& TrainingData, size_t n_features)
{
    W_encode = metric::PCA(TrainingData, n_features, averages);
    auto encoded = encode(TrainingData);
    W_decode = trans(W_encode); // computed once and saved
}

template <typename V>
blaze::DynamicMatrix<typename PCFA<V>::value_type> PCFA<V>::encode(const blaze::DynamicMatrix<PCFA<V>::value_type>& Data) {
    auto CenteredInput = blaze::DynamicMatrix<PCFA<V>::value_type>(Data.rows(), Data.columns(), 0);
    for (size_t col = 0; col < Data.columns(); col++)
        column(CenteredInput, col) = column(Data, col) - averages;
    return W_encode * CenteredInput;
    //return W_encode * Data;
}

template <typename V>
blaze::DynamicMatrix<typename PCFA<V>::value_type> PCFA<V>::decode(
        const blaze::DynamicMatrix<PCFA<V>::value_type>& Codes,
        bool unshift) {
    if (unshift) {
        auto Noncentered = W_decode * Codes;
        auto Centered = blaze::DynamicMatrix<typename PCFA<V>::value_type>(Noncentered.rows(), Noncentered.columns());
        for (size_t col = 0; col < Noncentered.columns(); col++)
            column(Centered, col) = column(Noncentered, col) + averages;
        return Centered;
    } else {
        return W_decode * Codes;
    }

}

template <typename V>
blaze::DynamicMatrix<typename PCFA<V>::value_type> PCFA<V>::average() {
    auto avg = blaze::DynamicMatrix<typename PCFA<V>::value_type>(averages.size(), 1);
    column(avg, 0) = averages;
    return avg;
    //return expand(averages, 1);  // expand absents in local version of Blaze-lib
}

template <typename V>
blaze::DynamicMatrix<typename PCFA<V>::value_type> PCFA<V>::eigenmodes() {
    auto Eigenmodes = blaze::DynamicMatrix<typename PCFA<V>::value_type>(W_decode.rows(), W_decode.columns() + 1);
    column(Eigenmodes, 0) = averages;
    submatrix(Eigenmodes, 0, 1, W_decode.rows(), W_decode.columns()) = W_decode;;;
    return Eigenmodes;
}

template <typename BlazeMatrix>
PCFA<typename BlazeMatrix::ElementType> PCFA_factory(const BlazeMatrix & TrainingData, size_t n_features)
{
    return PCFA<typename BlazeMatrix::ElementType>(TrainingData, n_features);
};

}  // namespace metric
#endif
