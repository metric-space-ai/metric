/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
//#ifndef _METRIC_MAPPING_PCFA_CPP
//#define _METRIC_MAPPING_PCFA_CPP

#include "PCFA.hpp"


namespace metric {



template <class BlazeMatrix>
blaze::DynamicMatrix<typename BlazeMatrix::ElementType> PCA_col(const BlazeMatrix & In, int n_components, blaze::DynamicVector<typename BlazeMatrix::ElementType> & averages)
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


template <class BlazeMatrix>
blaze::DynamicMatrix<typename BlazeMatrix::ElementType> PCA(
        const BlazeMatrix & In,
        int n_components,
        blaze::DynamicVector<typename BlazeMatrix::ElementType, blaze::rowVector> & averages)
{
    auto Result = blaze::DynamicMatrix<typename BlazeMatrix::ElementType>(n_components, In.columns(), 0);

    averages = blaze::sum<blaze::columnwise>(In) / In.rows();
    auto CenteredInput = blaze::DynamicMatrix<typename BlazeMatrix::ElementType>(In.rows(), In.columns(), 0);
    for (size_t row_idx = 0; row_idx < In.rows(); row_idx++)
        blaze::row(CenteredInput, row_idx) = blaze::row(In, row_idx) - averages;

    blaze::SymmetricMatrix<blaze::DynamicMatrix<typename BlazeMatrix::ElementType>> CovMat = blaze::evaluate(trans(CenteredInput) * CenteredInput);

    blaze::DynamicVector<typename BlazeMatrix::ElementType, blaze::columnVector> w(CovMat.rows());  // for eigenvalues
    blaze::DynamicMatrix<typename BlazeMatrix::ElementType, blaze::rowMajor> V(CovMat.rows(), CovMat.rows());  // for eigenvectors

    eigen(CovMat, w, V);

    // sort and select
    size_t lower_idx = 0;
    size_t upper_idx = w.size() - 1;

    int count = 0;
    while (count < n_components && upper_idx > lower_idx) {
        if (-w[lower_idx] > w[upper_idx]) {
            blaze::row(Result, count) = blaze::row(V, lower_idx); // add eigenpair
            lower_idx++;
        } else {
            blaze::row(Result, count) = blaze::row(V, upper_idx); // add eigenpair
            upper_idx--;
        }
        count++;
    }

    return Result; // eigenvectors in rows
}




// simple linear encoder based on PCA_col, accepts curves in columns
template <typename V>
PCFA_col<V>::PCFA_col(const blaze::DynamicMatrix<value_type>& TrainingData, size_t n_features)
{
    W_encode = metric::PCA_col(TrainingData, n_features, averages);
    //auto encoded = encode(TrainingData);
    W_decode = trans(W_encode); // computed once and saved
}

template <typename V>
blaze::DynamicMatrix<typename PCFA_col<V>::value_type> PCFA_col<V>::encode(const blaze::DynamicMatrix<PCFA_col<V>::value_type>& Data) {
    auto CenteredInput = blaze::DynamicMatrix<PCFA_col<V>::value_type>(Data.rows(), Data.columns(), 0);
    for (size_t col = 0; col < Data.columns(); col++)
        column(CenteredInput, col) = column(Data, col) - averages;
    return W_encode * CenteredInput;
    //return W_encode * Data;
}

template <typename V>
blaze::DynamicMatrix<typename PCFA_col<V>::value_type> PCFA_col<V>::decode(
        const blaze::DynamicMatrix<PCFA_col<V>::value_type>& Codes,
        bool unshift) {
    if (unshift) {
        auto Noncentered = W_decode * Codes;
        auto Centered = blaze::DynamicMatrix<typename PCFA_col<V>::value_type>(Noncentered.rows(), Noncentered.columns());
        for (size_t col = 0; col < Noncentered.columns(); col++)
            column(Centered, col) = column(Noncentered, col) + averages;
        return Centered;
    } else {
        return W_decode * Codes;
    }

}

template <typename V>
blaze::DynamicMatrix<typename PCFA_col<V>::value_type> PCFA_col<V>::average() {
    auto avg = blaze::DynamicMatrix<typename PCFA_col<V>::value_type>(averages.size(), 1);
    column(avg, 0) = averages;
    return avg;
    //return expand(averages, 1);  // expand absents in local version of Blaze-lib
}

template <typename V>
blaze::DynamicMatrix<typename PCFA_col<V>::value_type> PCFA_col<V>::eigenmodes() {
    auto Eigenmodes = blaze::DynamicMatrix<typename PCFA_col<V>::value_type>(W_decode.rows(), W_decode.columns() + 1);
    column(Eigenmodes, 0) = averages;
    submatrix(Eigenmodes, 0, 1, W_decode.rows(), W_decode.columns()) = W_decode;
    return Eigenmodes;
}




// simple linear encoder based on PCA, accepts curves in rows

template <typename recType, typename Metric>
PCFA<recType, Metric>::PCFA(const blaze::DynamicMatrix<value_type> & TrainingData, size_t n_features)
{
    W_decode = metric::PCA(TrainingData, n_features, averages);
    W_encode = trans(W_decode); // computed once and saved
}

template <typename recType, typename Metric>
PCFA<recType, Metric>::PCFA(std::vector<recType> & TrainingData, size_t n_features)
{
    blaze::DynamicMatrix<value_type> blaze_in(TrainingData.size(), TrainingData[0].size(), 0);
    for (size_t i = 0; i < TrainingData.size(); ++i) // TODO optimize using iterators!! // TODO replace with call of converter
        for (size_t j = 0; j < TrainingData[0].size(); ++j)
            blaze_in(i, j) = TrainingData[i][j];
    blaze::DynamicVector<value_type, blaze::rowVector> avgs;

    W_decode = metric::PCA(blaze_in, 4, avgs);
    W_encode = trans(W_decode); // computed once and saved
}


template <typename recType, typename Metric>
blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>
PCFA<recType, Metric>::encode(
        const blaze::DynamicMatrix<PCFA<recType, Metric>::value_type> & Data) {
    auto CenteredInput = blaze::DynamicMatrix<PCFA<recType, Metric>::value_type>(Data.rows(), Data.columns(), 0);
    for (size_t row_idx = 0; row_idx < Data.rows(); row_idx++)
        blaze::row(CenteredInput, row_idx) = blaze::row(Data, row_idx) - averages;
    return CenteredInput * W_encode;
}



template <typename recType, typename Metric>
std::vector<recType>
PCFA<recType, Metric>::encode(const std::vector<recType> & Data) {
    auto DataBlaze = vector_to_blaze(Data);
    auto CenteredInput = blaze::DynamicMatrix<PCFA<recType, Metric>::value_type>(DataBlaze.rows(), DataBlaze.columns(), 0);
    for (size_t row_idx = 0; row_idx < DataBlaze.rows(); row_idx++)
        blaze::row(CenteredInput, row_idx) = blaze::row(DataBlaze, row_idx) - averages;
    blaze::DynamicMatrix<PCFA<recType, Metric>::value_type> Out = CenteredInput * W_encode;
    return blaze_to_vector<recType>(Out);
}


template <typename recType, typename Metric>
blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>
PCFA<recType, Metric>::decode(
        const blaze::DynamicMatrix<PCFA<recType, Metric>::value_type>& Codes,
        bool unshift) {
    if (unshift) {
        auto Noncentered = Codes * W_decode;
        auto Centered = blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>(Noncentered.rows(), Noncentered.columns());
        for (size_t row_idx = 0; row_idx < Noncentered.rows(); row_idx++)
            blaze::row(Centered, row_idx) = blaze::row(Noncentered, row_idx) + averages;
        return Centered;
    } else {
        return Codes * W_decode;
    }

}



template <typename recType, typename Metric>
std::vector<recType>
PCFA<recType, Metric>::decode(
        const std::vector<recType> & Codes,
        bool unshift) {
    blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type> Out;  // TODO optimize
    auto CodesBlaze = vector_to_blaze(Codes);
    if (unshift) {
        auto Noncentered = CodesBlaze * W_decode;
        blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type> Centered = blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>(Noncentered.rows(), Noncentered.columns());
        for (size_t row_idx = 0; row_idx < Noncentered.rows(); row_idx++)
            blaze::row(Centered, row_idx) = blaze::row(Noncentered, row_idx) + averages;
        Out = Centered;
        //return blaze_to_vector(Out);
    } else {
        Out = CodesBlaze * W_decode;
        //return blaze_to_vector(Out);
    }
    return blaze_to_vector<recType>(Out);

}


template <typename recType, typename Metric>
blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>
PCFA<recType, Metric>::average() {
    auto avg = blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>(1, averages.size());
    blaze::row(avg, 0) = averages;
    return avg;
    //return expand(averages, 0);  // expand absents in local version of Blaze-lib
}

template <typename recType, typename Metric>
blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>
PCFA<recType, Metric>::eigenmodes() {
    auto Eigenmodes = blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>(W_decode.rows() + 1, W_decode.columns());
    blaze::row(Eigenmodes, 0) = averages;
    submatrix(Eigenmodes, 1, 0, W_decode.rows(), W_decode.columns()) = W_decode;
    return Eigenmodes;
}


template <typename recType, typename Metric>
blaze::DynamicMatrix<typename PCFA<recType, Metric>::value_type>
PCFA<recType, Metric>::vector_to_blaze(const std::vector<recType> & In) {
    blaze::DynamicMatrix<value_type> Out(In.size(), In[0].size(), 0);
    for (size_t i = 0; i < In.size(); ++i) // TODO optimize using iterators!!
        for (size_t j = 0; j < In[0].size(); ++j)
            Out(i, j) = In[i][j];
    return Out;
}


template <typename recType, typename Metric>
template <typename R>
typename std::enable_if <
 std::is_same<
  R,
  std::vector<typename PCFA<R, Metric>::value_type>
 >::value,
 std::vector<R>
>::type // here we support only STL vector
PCFA<recType, Metric>::blaze_to_vector(const blaze::DynamicMatrix<typename PCFA<R, Metric>::value_type> & In) { // TODO support arbitrary type!
    std::vector<recType> Out;
    for (size_t i = 0; i < In.rows(); ++i) {  // TODO optimize using iterators!!
        recType rec;
        for (size_t j = 0; j < In[0].size(); ++j)
            rec.push_back(In[i][j]);
        Out.push_back(rec);
    }
    return Out;
}


template <typename recType, typename Metric>
template <typename R>
typename std::enable_if<
 std::is_same<
  R,
  blaze::DynamicVector<typename PCFA<R, Metric>::value_type, blaze::rowVector>
 >::value,
 std::vector<R>
>::type
PCFA<recType, Metric>::blaze_to_vector(const blaze::DynamicMatrix<typename PCFA<R, Metric>::value_type> & In) { // only blaze row-vector
    std::vector<recType> Out;
    for (size_t i = 0; i < In.rows(); ++i) {  // TODO optimize using iterators!!
        recType rec(In.rows()); // blaze specific
        for (size_t j = 0; j < In.columns(); ++j)
            rec[j] = In(i, j);  // blaze specific
        Out.push_back(rec);
    }
    return Out;
}


template <typename BlazeMatrix>
PCFA_col<typename BlazeMatrix::ElementType> PCFA_col_factory(const BlazeMatrix & TrainingData, size_t n_features)
{
    return PCFA_col<typename BlazeMatrix::ElementType>(TrainingData, n_features);
};


//template <typename BlazeMatrix>
//PCFA<typename BlazeMatrix::ElementType> PCFA_factory(const BlazeMatrix & TrainingData, size_t n_features)
//{
//    return PCFA<typename BlazeMatrix::ElementType>(TrainingData, n_features);
//}; // TODO make factory for any Blaze type


template <typename ElementType>
PCFA<blaze::DynamicMatrix<ElementType>, void> PCFA_factory(
        blaze::DynamicMatrix<ElementType> in,
        size_t n_features) // special factory for Blaze matrix
{
    return PCFA<blaze::DynamicMatrix<ElementType>, void>(in, n_features);
}


}  // namespace metric
//#endif
