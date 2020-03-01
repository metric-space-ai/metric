/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_ESN_CPP
#define _METRIC_MAPPING_ESN_CPP
#include <cassert>
#include "ESN.hpp"

namespace metric {

namespace ESN_details {

    // ---------------------------------  math functions:

    template <typename T>
    blaze::DynamicMatrix<T> get_readout_no_echo(  // for ESN with disables echo
        const blaze::DynamicMatrix<T>& Slices, const blaze::DynamicMatrix<T>& W_in);

    template <typename T>
    blaze::DynamicMatrix<T> get_readout(  // echo mode
        const blaze::DynamicMatrix<T>& Slices, const blaze::DynamicMatrix<T>& W_in,
        const blaze::CompressedMatrix<T>& W,  // TODO make sparse
        T alpha = 0.5, size_t washout = 0);

    template <typename T>
    blaze::DynamicMatrix<T> ridge(
        const blaze::DynamicMatrix<T>& Target, const blaze::DynamicMatrix<T>& Readout, T beta = 0.5);

    // ---------------------------------  math functions:

    template <typename T>
    blaze::DynamicMatrix<T> get_readout_no_echo(  // for ESN with disables echo
        const blaze::DynamicMatrix<T>& Slices, const blaze::DynamicMatrix<T>& W_in)
    {
        size_t slice_size = W_in.columns();
        assert(slice_size > 0);
        slice_size = slice_size - 1;  // size of input vector. first column is for offset and will be used separately

        assert(Slices.rows() == slice_size);

        auto W_in_submatrix = submatrix(W_in, 0UL, 1UL, W_in.rows(), slice_size);  // all except first column
        auto w_in_offset = submatrix(W_in, 0UL, 0UL, W_in.rows(), 1UL);  // first column
        auto Ones = blaze::DynamicMatrix<T>(w_in_offset.columns(), Slices.columns(), 1);

        return evaluate(tanh(W_in_submatrix * Slices + w_in_offset * Ones));
    }

    template <typename T>
    blaze::DynamicMatrix<T> get_readout(  // echo mode
        const blaze::DynamicMatrix<T>& Slices, const blaze::DynamicMatrix<T>& W_in,
        const blaze::CompressedMatrix<T>& W,  // TODO make sparse
        T alpha,  // = 0.5,
        size_t washout  // = 0
    )
    {

        if (washout == 0 && alpha == 1)  // if echo disabled, we run faster overload without sample loop
            return get_readout_no_echo(Slices, W_in);

        size_t x_size = W.rows();  // size of inter-step echo buffer
        assert(x_size == W.columns());  // W must be square

        size_t slice_size = W_in.columns();
        assert(slice_size > 0);
        slice_size = slice_size - 1;  // size of input vector. first column is for offset and will be used separately

        assert(Slices.rows() == slice_size);

        auto x = blaze::DynamicMatrix<T>(
            x_size, 1UL, 0.0);  // matrix type is due to impossibility of addition of matrix and vector
        // TODO set initial random values, if needed

        //  here we only define symbolyc expressions, all computations are deferred
        auto W_in_submatrix = submatrix(W_in, 0UL, 1UL, W_in.rows(), slice_size);  // all except first column
        auto w_in_offset = submatrix(W_in, 0UL, 0UL, W_in.rows(), 1UL);  // first column

        size_t n = 0;
        blaze::DynamicMatrix<T> current_slice
            = columns(Slices, { n });  // will be updated inside loop. Type is set in order to force evaluation
        // TODO refactor: try n of vector type
        auto input_summand = W_in_submatrix * current_slice
            + w_in_offset;  // define all symbolic expressions out of loop (but dependent on n ones must be updated)
        auto x_prev_summand = W * x;

        assert(Slices.columns() > washout);  // TODO consider >= x_size in order to avoid undetermined system
        auto Output = blaze::DynamicMatrix<T>(x_size, Slices.columns() - washout);

        for (n = 0; n < Slices.columns(); n++) {
            current_slice
                = columns(Slices, { n });  // update for each n // TODO consider making n also Blaze expression?
            x = evaluate(tanh(input_summand + x_prev_summand) * alpha + x * (1 - alpha));

            if (n >= washout)
                columns(Output, { n - washout }) = x;  // we output all readout
        }

        return Output;
    }

    template <typename T>
    blaze::DynamicMatrix<T> ridge(
        const blaze::DynamicMatrix<T>& Target, const blaze::DynamicMatrix<T>& Readout,
        T beta  // = 0.5
    )
    {
        auto I = blaze::IdentityMatrix<T>(Readout.rows());
        auto TR = trans(Readout);
        return Target * TR * inv(Readout * TR + I * beta);
    }

}  // namespace ESN_details

template <typename recType, typename Metric>
void ESN<recType, Metric>::create_W(size_t w_size, value_type w_connections, value_type w_sr)
{
    W = blaze::CompressedMatrix<value_type>(w_size, w_size, 0.0);  // TODO make sparse

    if (w_sr > 0) {
        // TODO disable the following W fullfilling code if considered to use graph
        auto uniform_int = std::uniform_int_distribution<int>(0, w_size - 1);
        auto uniform_value_type = std::uniform_real_distribution<value_type>(-1, 1);
        int count;
        size_t r_row, r_col;
        for (r_col = 0; r_col < W.columns(); r_col++)
            for (count = 0; count < w_connections; count++) {
                r_row = uniform_int(rgen);
                if (W.find(r_row, r_col) == W.end(r_row))  // find(..) works for compressed matrix only
                    W.insert(r_row, r_col, uniform_value_type(rgen));
                else
                    count--;  // retry
            }

        // spectral radius normalization
        blaze::DynamicVector<blaze::complex<value_type>, blaze::columnVector> eig(0UL, 0.0);

        blaze::DynamicMatrix<value_type> Wd
            = W;  // DynamicMatrix needed for eigen call // TODO disable this line if considered to use graph
        // TODO enable the following 3 lines if considered to use graph
        //auto graph = metric::graph::RandomUniform<double, false>(w_size, -1, 1, w_connections);
        //blaze::DynamicMatrix<double> Wd = graph.get_matrix(); // DynamicMAtrix needed for eigen call
        //W = Wd;

        eigen(Wd, eig);
        auto sr = max(sqrt(pow(real(eig), 2) + pow(imag(eig), 2)));
        W = W * w_sr / sr;
    }
}

template <typename recType, typename Metric>
ESN<recType, Metric>::ESN(size_t w_size,  // = 500, // number of elements in reservoir
    value_type w_connections,  // = 10, // number of interconnections (for each reservoir element)
    value_type w_sr,  // = 0.6, // desired spectral radius of the reservoir
    value_type alpha_,  // = 0.5, // leak rate, number of slices excluded from output for washout
    size_t washout_,  // = 1,
    value_type beta_  // = 0.5, // ridge solver metaparameter
    )
    : alpha(alpha_)
    , beta(beta_)
    , washout(washout_)
{
    assert(w_connections / (float)w_size < 0.5);
    create_W(w_size, w_connections, w_sr);
}

template <typename recType, typename Metric>
void ESN<recType, Metric>::train(const blaze::DynamicMatrix<value_type>& Slices, const blaze::DynamicMatrix<value_type>& Target)
{

    size_t in_size = Slices.rows();

    auto uniform_double = std::uniform_real_distribution<value_type>(-1, 1);
    size_t r_row, r_col;
    W_in = blaze::DynamicMatrix<value_type>(W.rows(), in_size + 1, 0.0);
    for (r_row = 0; r_row < W.rows(); r_row++)
        for (r_col = 0; r_col <= in_size; r_col++) {
            assert(r_row < W_in.rows() && r_col < W_in.columns());
            W_in(r_row, r_col) = uniform_double(rgen);
        }

    blaze::DynamicMatrix<value_type> Readout = ESN_details::get_readout(Slices,  // input signal
        W_in,  // input weights
        W,  // reservoir internal weights (square matrix)
        alpha, washout  // leak rate, number of slices excluded from output for washout
    );

    blaze::DynamicMatrix<value_type> target_submat = submatrix(Target, 0UL, washout, Target.rows(), Target.columns() - washout);
    W_out = ESN_details::ridge(target_submat, Readout, beta);
    trained = true;
}

template <typename recType, typename Metric>
void ESN<recType, Metric>::train(const std::vector<recType> & Slices, const std::vector<recType> & Target)
{
    auto SlicesMat = vector_to_blaze(Slices);
    auto TargetMat = vector_to_blaze(Target);
    train(SlicesMat, TargetMat);
}



template <typename recType, typename Metric>
blaze::DynamicMatrix<typename ESN<recType, Metric>::value_type> ESN<recType, Metric>::predict(const blaze::DynamicMatrix<value_type>& Slices)
{
    assert(trained);

    blaze::DynamicMatrix<value_type> Readout = ESN_details::get_readout(Slices,  // input signal
        W_in,  // input weights
        W,  // reservoir internal weights (square matrix)
        alpha, washout  // leak rate, number of slices excluded from output for washout
    );

    return W_out * Readout;
}

template <typename recType, typename Metric>
std::vector<recType> ESN<recType, Metric>::predict(const std::vector<recType>& Slices)
{
    auto SlicesMat = vector_to_blaze(Slices);
    auto PredictionMat = predict(SlicesMat);
    return blaze2rectype<recType>(PredictionMat);
}


template <typename recType, typename Metric>
blaze::DynamicMatrix<typename ESN<recType, Metric>::value_type>
ESN<recType, Metric>::vector_to_blaze(const std::vector<recType> & In) {
    blaze::DynamicMatrix<value_type> Out(In[0].size(), In.size(), 0);  // transpose
    for (size_t i = 0; i < In.size(); ++i) // TODO optimize by using iterators
        for (size_t j = 0; j < In[0].size(); ++j)
            Out(j, i) = In[i][j];
    return Out;
}


template <typename recType, typename Metric>
template <typename R>
typename std::enable_if <
 determine_container_type<R>::code == 1,
 std::vector<R>
>::type
ESN<recType, Metric>::blaze2rectype(const blaze::DynamicMatrix<typename ESN<R, Metric>::value_type> & In) {
    std::vector<recType> Out;
    for (size_t i = 0; i < In.columns(); ++i) {
        recType rec;
        for (size_t j = 0; j < In.rows(); ++j)
            rec.push_back(In(j, i)); // transpose
        Out.push_back(rec);
    }
    return Out;
}


template <typename recType, typename Metric>
template <typename R>
typename std::enable_if<
 determine_container_type<R>::code == 2,
 std::vector<R>
>::type
ESN<recType, Metric>::blaze2rectype(const blaze::DynamicMatrix<typename ESN<R, Metric>::value_type> & In) { // only blaze row-vector
    std::vector<recType> Out;
    for (size_t i = 0; i < In.columns(); ++i) {
        recType rec(In.rows()); // blaze specific
        for (size_t j = 0; j < In.rows(); ++j)
            rec[j] = In(j, i);  // blaze specific  // transpose
        Out.push_back(rec);
    }
    return Out;
}



//template <typename recType, typename Metric>
//blaze::DynamicMatrix<typename ESN<recType, Metric>::value_type>
//ESN<recType, Metric>::vector_to_blaze(const std::vector<recType> & In) {
//    blaze::DynamicMatrix<value_type> Out(In.size(), In[0].size(), 0);
//    for (size_t i = 0; i < In.size(); ++i) // TODO optimize by using iterators
//        for (size_t j = 0; j < In[0].size(); ++j)
//            Out(i, j) = In[i][j];
//    return Out;
//}



//template <typename recType, typename Metric>
//template <typename R>
//typename std::enable_if <
// determine_container_type<R>::code == 1,
// std::vector<R>
//>::type
//ESN<recType, Metric>::blaze2rectype(const blaze::DynamicMatrix<typename ESN<R, Metric>::value_type> & In) {
//    std::vector<recType> Out;
//    for (size_t i = 0; i < In.rows(); ++i) {
//        recType rec;
//        for (size_t j = 0; j < In.columns(); ++j)
//            rec.push_back(In(i, j));
//        Out.push_back(rec);
//    }
//    return Out;
//}


//template <typename recType, typename Metric>
//template <typename R>
//typename std::enable_if<
// determine_container_type<R>::code == 2,
// std::vector<R>
//>::type
//ESN<recType, Metric>::blaze2rectype(const blaze::DynamicMatrix<typename ESN<R, Metric>::value_type> & In) { // only blaze row-vector
//    std::vector<recType> Out;
//    for (size_t i = 0; i < In.rows(); ++i) {
//        recType rec(In.rows()); //  .columns() ?? // blaze specific
//        for (size_t j = 0; j < In.columns(); ++j)
//            rec[j] = In(i, j);  // blaze specific
//        Out.push_back(rec);
//    }
//    return Out;
//}






}  // namespace metric
#endif  // header guard
