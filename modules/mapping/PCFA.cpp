/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_MAPPING_PCFA_CPP
#define _METRIC_MAPPING_PCFA_CPP

#include "PCFA.hpp"

//#include "modules/utils/visualizer.hpp" // TODO remove

namespace metric {

template <class BlazeMatrix>
blaze::DynamicMatrix<double> PCA(const BlazeMatrix & In, int n_components, blaze::DynamicVector<double> & averages)
{
    auto Result = blaze::DynamicMatrix<double>(n_components, In.rows(), 0);

    averages = blaze::sum<blaze::rowwise>(In) / In.columns();
    auto CenteredInput = blaze::DynamicMatrix<double>(In.rows(), In.columns(), 0);
    for (size_t col = 0; col < In.columns(); col++)
        column(CenteredInput, col) = column(In, col) - averages;

    blaze::SymmetricMatrix<blaze::DynamicMatrix<double>> CovMat = blaze::evaluate(CenteredInput * trans(CenteredInput));

    blaze::DynamicVector<double, blaze::columnVector> w(In.rows());  // for eigenvalues
    blaze::DynamicMatrix<double, blaze::rowMajor> V(In.rows(), In.rows());  // for eigenvectors

    eigen(CovMat, w, V);

    for (size_t row = 0; row < V.rows(); row++)
        blaze::row(V, row) = blaze::row(V, row) * w[row];

    // sort and select
    size_t lower_idx = 0;
    size_t upper_idx = w.size() - 1;
    double spectral_radius;  // also we get spectral radius for normalization: we process the first eigenvalue specially
    if ((-w[lower_idx] > w[upper_idx]))
        spectral_radius = w[lower_idx];
    else
        spectral_radius = w[upper_idx];
    int count = 0;
    while (count < n_components && upper_idx > lower_idx) {
        if (-w[lower_idx] > w[upper_idx]) {
            blaze::row(Result, count) = blaze::row(V, lower_idx) / spectral_radius;  // add eigenpair
            lower_idx++;
        } else {
            blaze::row(Result, count) = blaze::row(V, upper_idx) / spectral_radius;  // add eigenpair
            upper_idx--;
        }
        count++;
    }

    return Result;
}

// simple linear encoder based on PCA

PCFA::PCFA() {}

void PCFA::train(const blaze::DynamicMatrix<double>& Slices, size_t n_features)
{
    W_encode = metric::PCA(Slices, n_features, averages);
    auto encoded = encode(Slices);
    W_decode = trans(W_encode);
}

blaze::DynamicMatrix<double> PCFA::encode(const blaze::DynamicMatrix<double>& Slices) {
    return W_encode * Slices;
}

blaze::DynamicMatrix<double> PCFA::decode(const blaze::DynamicMatrix<double>& Codes, bool unshift) {
    if (unshift) {
        auto Noncentered = W_decode * Codes;
        auto Centered = blaze::DynamicMatrix<double>(Noncentered.rows(), Noncentered.columns());
        for (size_t col = 0; col < Noncentered.columns(); col++)
            column(Centered, col) = column(Noncentered, col) - averages;
        return Centered;
    } else {
        return W_decode * Codes;
    }

}

}  // namespace metric
#endif
