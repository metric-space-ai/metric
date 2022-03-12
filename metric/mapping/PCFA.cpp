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
blaze::DynamicMatrix<typename BlazeMatrix::ElementType>
PCA(const BlazeMatrix &In, int n_components,
	blaze::DynamicVector<typename BlazeMatrix::ElementType, blaze::rowVector> &averages)
{
	auto Result = blaze::DynamicMatrix<typename BlazeMatrix::ElementType>(n_components, In.columns(), 0);

	averages = blaze::sum<blaze::columnwise>(In) / In.rows();
	auto CenteredInput = blaze::DynamicMatrix<typename BlazeMatrix::ElementType>(In.rows(), In.columns(), 0);
	for (size_t row_idx = 0; row_idx < In.rows(); row_idx++)
		blaze::row(CenteredInput, row_idx) = blaze::row(In, row_idx) - averages;

	if (n_components < 1)
		return Result;

	blaze::SymmetricMatrix<blaze::DynamicMatrix<typename BlazeMatrix::ElementType>> CovMat =
		blaze::evaluate(trans(CenteredInput) * CenteredInput);

	blaze::DynamicVector<typename BlazeMatrix::ElementType, blaze::columnVector> w(CovMat.rows()); // for eigenvalues
	blaze::DynamicMatrix<typename BlazeMatrix::ElementType, blaze::rowMajor> V(CovMat.rows(),
																			   CovMat.rows()); // for eigenvectors

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

// simple linear encoder based on PCA, accepts curves in rows

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const blaze::DynamicMatrix<value_type> &TrainingData, const size_t n_features)
{
	W_decode = metric::PCA(TrainingData, n_features, averages);
	W_encode = trans(W_decode); // computed once and saved
}

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const std::vector<RecType> &TrainingData, const size_t n_features)
{
	blaze::DynamicMatrix<value_type> blaze_in(TrainingData.size(), TrainingData[0].size(), 0);
	for (size_t i = 0; i < TrainingData.size();
		 ++i) // TODO optimize using iterators!! // TODO replace with call of converter
		for (size_t j = 0; j < TrainingData[0].size(); ++j)
			blaze_in(i, j) = TrainingData[i][j];
	W_decode = metric::PCA(blaze_in, n_features, averages);
	W_encode = trans(W_decode); // computed once and saved
}

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const blaze::DynamicMatrix<value_type> &Weights,
							const blaze::DynamicVector<value_type, blaze::rowVector> &avgs)
{
	W_decode = Weights;
	W_encode = trans(W_decode); // computed once and saved
	averages = avgs;
}

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const std::vector<RecType> &Weights, const RecType &avgs)
{
	W_decode = blaze::DynamicMatrix<value_type>(Weights.size(), Weights[0].size(), 0);
	for (size_t i = 0; i < Weights.size(); ++i)
		for (size_t j = 0; j < Weights[0].size(); ++j)
			W_decode(i, j) = Weights[i][j];
	W_encode = trans(W_decode); // computed once and saved
	averages = blaze::DynamicVector<value_type, blaze::rowVector>(avgs.size(), 0);
	for (size_t i = 0; i < avgs.size(); ++i) {
		averages[i] = avgs[i];
	}
}

template <typename RecType, typename Metric>
blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>
PCFA<RecType, Metric>::encode(const blaze::DynamicMatrix<PCFA<RecType, Metric>::value_type> &Data)
{
	auto CenteredInput = blaze::DynamicMatrix<PCFA<RecType, Metric>::value_type>(Data.rows(), Data.columns(), 0);
	for (size_t row_idx = 0; row_idx < Data.rows(); row_idx++)
		blaze::row(CenteredInput, row_idx) = blaze::row(Data, row_idx) - averages;
	return CenteredInput * W_encode;
}

template <typename RecType, typename Metric>
std::vector<RecType> PCFA<RecType, Metric>::encode(const std::vector<RecType> &Data)
{
	auto DataBlaze = vector_to_blaze(Data);
	auto CenteredInput =
		blaze::DynamicMatrix<PCFA<RecType, Metric>::value_type>(DataBlaze.rows(), DataBlaze.columns(), 0);
	for (size_t row_idx = 0; row_idx < DataBlaze.rows(); row_idx++)
		blaze::row(CenteredInput, row_idx) = blaze::row(DataBlaze, row_idx) - averages;
	blaze::DynamicMatrix<PCFA<RecType, Metric>::value_type> Out = CenteredInput * W_encode;
	return blaze2RecType<RecType>(Out);
}

template <typename RecType, typename Metric>
blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>
PCFA<RecType, Metric>::decode(const blaze::DynamicMatrix<PCFA<RecType, Metric>::value_type> &Codes, const bool unshift)
{
	if (unshift) {
		auto Noncentered = Codes * W_decode;
		auto Centered =
			blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(Noncentered.rows(), Noncentered.columns());
		for (size_t row_idx = 0; row_idx < Noncentered.rows(); row_idx++)
			blaze::row(Centered, row_idx) = blaze::row(Noncentered, row_idx) + averages;
		return Centered;
	} else {
		return Codes * W_decode;
	}
}

template <typename RecType, typename Metric>
std::vector<RecType> PCFA<RecType, Metric>::decode(const std::vector<RecType> &Codes, const bool unshift)
{
	auto CodesBlaze = vector_to_blaze(Codes);
	if (unshift) {
		auto Noncentered = CodesBlaze * W_decode;
		auto Centered =
			blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(Noncentered.rows(), Noncentered.columns());
		for (size_t row_idx = 0; row_idx < Noncentered.rows(); row_idx++)
			blaze::row(Centered, row_idx) = blaze::row(Noncentered, row_idx) + averages;
		return blaze2RecType<RecType>(Centered);
	} else {
		return blaze2RecType<RecType>(CodesBlaze * W_decode);
	}
}

template <typename RecType, typename Metric>
blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type> PCFA<RecType, Metric>::average_mat()
{
	auto avg = blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(1, averages.size());
	blaze::row(avg, 0) = averages;
	return avg;
}

template <typename RecType, typename Metric> RecType PCFA<RecType, Metric>::average()
{
	blaze::DynamicMatrix<value_type> result(1, averages.size());
	blaze::row(result, 0) = averages;
	return blaze2RecType<RecType>(result)[0];
}

template <typename RecType, typename Metric> std::vector<RecType> PCFA<RecType, Metric>::weights()
{
	return blaze2RecType<RecType>(W_decode);
}

template <typename RecType, typename Metric>
blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type> PCFA<RecType, Metric>::eigenmodes_mat()
{
	auto Eigenmodes =
		blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(W_decode.rows() + 1, W_decode.columns());
	blaze::row(Eigenmodes, 0) = averages;
	submatrix(Eigenmodes, 1, 0, W_decode.rows(), W_decode.columns()) = W_decode;
	return Eigenmodes;
}


template <typename RecType, typename Metric> std::vector<RecType> PCFA<RecType, Metric>::eigenmodes()
{
	return blaze2RecType<RecType>(eigenmodes_mat());
}

template <typename RecType, typename Metric>
blaze::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>
PCFA<RecType, Metric>::vector_to_blaze(const std::vector<RecType> &In)
{
	blaze::DynamicMatrix<value_type> Out(In.size(), In[0].size(), 0);
	for (size_t i = 0; i < In.size(); ++i) // TODO optimize using iterators!!
		for (size_t j = 0; j < In[0].size(); ++j)
			Out(i, j) = In[i][j];
	return Out;
}

template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if<determine_container_type<R>::code == 1,
						std::vector<R>>::type // here we support only STL vector
PCFA<RecType, Metric>::blaze2RecType(const blaze::DynamicMatrix<typename PCFA<R, Metric>::value_type> &In)
{ // TODO support arbitrary type!
	std::vector<RecType> Out;
	for (size_t i = 0; i < In.rows(); ++i) { // TODO optimize using iterators!!
		RecType rec;
		for (size_t j = 0; j < In.columns(); ++j)
			rec.push_back(In(i, j));
		Out.push_back(rec);
	}
	return Out;
}

template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if<determine_container_type<R>::code == 2, std::vector<R>>::type
PCFA<RecType, Metric>::blaze2RecType(const blaze::DynamicMatrix<typename PCFA<R, Metric>::value_type> &In)
{ // only blaze row-vector
	std::vector<RecType> Out;
	for (size_t i = 0; i < In.rows(); ++i) { // TODO optimize using iterators!!
		RecType rec(In.columns());			 // blaze specific
		for (size_t j = 0; j < In.columns(); ++j)
			rec[j] = In(i, j); // blaze specific
		Out.push_back(rec);
	}
	return Out;
}

template <typename ElementType>
PCFA<blaze::DynamicMatrix<ElementType>, void> PCFA_factory(blaze::DynamicMatrix<ElementType> TrainingData,
														   size_t n_features) // special factory for Blaze matrix
{
	return PCFA<blaze::DynamicMatrix<ElementType>, void>(TrainingData, n_features);
}

template <template <typename, typename> class Container, typename ValueType, typename Allocator>
PCFA<Container<ValueType, Allocator>, void> PCFA_factory(std::vector<Container<ValueType, Allocator>> &TrainingData,
														 size_t n_features)
{
	return PCFA<Container<ValueType, Allocator>, void>(TrainingData, n_features);
}

template <template <typename, bool> class Container, typename ValueType, bool F>
PCFA<Container<ValueType, F>, void> PCFA_factory(std::vector<Container<ValueType, F>> &TrainingData, size_t n_features)
{
	return PCFA<Container<ValueType, F>, void>(TrainingData, n_features);
}

} // namespace metric
#endif
