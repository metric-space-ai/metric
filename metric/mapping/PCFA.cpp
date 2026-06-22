/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_MAPPING_PCFA_CPP
#define _METRIC_MAPPING_PCFA_CPP

#include "PCFA.hpp"

namespace mtrc {

template <class Matrix>
mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>
PCA_col(const Matrix &In, int n_components, mtrc::numeric::DynamicVector<typename Matrix::ElementType> &averages)
{
	auto Result = mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>(n_components, In.rows(), 0);

	averages = mtrc::numeric::sum<mtrc::numeric::rowwise>(In) / In.columns();
	auto CenteredInput = mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>(In.rows(), In.columns(), 0);
	for (size_t col = 0; col < In.columns(); col++)
		column(CenteredInput, col) = column(In, col) - averages;

	mtrc::numeric::SymmetricMatrix<mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>> CovMat =
		mtrc::numeric::evaluate(CenteredInput * trans(CenteredInput));

	mtrc::numeric::DynamicVector<typename Matrix::ElementType, mtrc::numeric::columnVector> w(
		In.rows()); // for eigenvalues
	mtrc::numeric::DynamicMatrix<typename Matrix::ElementType, mtrc::numeric::rowMajor> V(
		In.rows(),
		In.rows()); // for eigenvectors

	eigen(CovMat, w, V);

	// sort and select
	size_t lower_idx = 0;
	size_t upper_idx = w.size() - 1;
	int count = 0;
	while (count < n_components && upper_idx > lower_idx) {
		if (-w[lower_idx] > w[upper_idx]) {
			mtrc::numeric::row(Result, count) = mtrc::numeric::row(V, lower_idx); // add eigenpair
			lower_idx++;
		} else {
			mtrc::numeric::row(Result, count) = mtrc::numeric::row(V, upper_idx); // add eigenpair
			upper_idx--;
		}
		count++;
	}

	return Result;
}

template <class Matrix>
mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>
PCA(const Matrix &In, int n_components,
	mtrc::numeric::DynamicVector<typename Matrix::ElementType, mtrc::numeric::rowVector> &averages)
{
	auto Result = mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>(n_components, In.columns(), 0);

	averages = mtrc::numeric::sum<mtrc::numeric::columnwise>(In) / In.rows();
	auto CenteredInput = mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>(In.rows(), In.columns(), 0);
	for (size_t row_idx = 0; row_idx < In.rows(); row_idx++)
		mtrc::numeric::row(CenteredInput, row_idx) = mtrc::numeric::row(In, row_idx) - averages;

	if (n_components < 1)
		return Result;

	mtrc::numeric::SymmetricMatrix<mtrc::numeric::DynamicMatrix<typename Matrix::ElementType>> CovMat =
		mtrc::numeric::evaluate(trans(CenteredInput) * CenteredInput);

	mtrc::numeric::DynamicVector<typename Matrix::ElementType, mtrc::numeric::columnVector> w(
		CovMat.rows()); // for eigenvalues
	mtrc::numeric::DynamicMatrix<typename Matrix::ElementType, mtrc::numeric::rowMajor> V(
		CovMat.rows(),
		CovMat.rows()); // for eigenvectors

	eigen(CovMat, w, V);

	// sort and select
	size_t lower_idx = 0;
	size_t upper_idx = w.size() - 1;

	int count = 0;
	while (count < n_components && upper_idx > lower_idx) {
		if (-w[lower_idx] > w[upper_idx]) {
			mtrc::numeric::row(Result, count) = mtrc::numeric::row(V, lower_idx); // add eigenpair
			lower_idx++;
		} else {
			mtrc::numeric::row(Result, count) = mtrc::numeric::row(V, upper_idx); // add eigenpair
			upper_idx--;
		}
		count++;
	}

	return Result; // eigenvectors in rows
}

// simple linear encoder based on PCA_col, accepts curves in columns
template <typename V>
PCFA_col<V>::PCFA_col(const mtrc::numeric::DynamicMatrix<value_type> &TrainingData, size_t n_features)
{
	W_encode = mtrc::PCA_col(TrainingData, n_features, averages);
	W_decode = trans(W_encode); // computed once and saved
}

template <typename V>
mtrc::numeric::DynamicMatrix<typename PCFA_col<V>::value_type>
PCFA_col<V>::encode(const mtrc::numeric::DynamicMatrix<PCFA_col<V>::value_type> &Data)
{
	auto CenteredInput = mtrc::numeric::DynamicMatrix<PCFA_col<V>::value_type>(Data.rows(), Data.columns(), 0);
	for (size_t col = 0; col < Data.columns(); col++)
		column(CenteredInput, col) = column(Data, col) - averages;
	return W_encode * CenteredInput;
	// return W_encode * Data;
}

template <typename V>
mtrc::numeric::DynamicMatrix<typename PCFA_col<V>::value_type>
PCFA_col<V>::decode(const mtrc::numeric::DynamicMatrix<PCFA_col<V>::value_type> &Codes, bool unshift)
{
	if (unshift) {
		auto Noncentered = W_decode * Codes;
		auto Centered =
			mtrc::numeric::DynamicMatrix<typename PCFA_col<V>::value_type>(Noncentered.rows(), Noncentered.columns());
		for (size_t col = 0; col < Noncentered.columns(); col++)
			column(Centered, col) = column(Noncentered, col) + averages;
		return Centered;
	} else {
		return W_decode * Codes;
	}
}

template <typename V> mtrc::numeric::DynamicMatrix<typename PCFA_col<V>::value_type> PCFA_col<V>::average()
{
	auto avg = mtrc::numeric::DynamicMatrix<typename PCFA_col<V>::value_type>(averages.size(), 1);
	column(avg, 0) = averages;
	return avg;
	// return expand(averages, 1);  // expand absents in local version of Numeric-lib
}

template <typename V> mtrc::numeric::DynamicMatrix<typename PCFA_col<V>::value_type> PCFA_col<V>::eigenmodes()
{
	auto Eigenmodes =
		mtrc::numeric::DynamicMatrix<typename PCFA_col<V>::value_type>(W_decode.rows(), W_decode.columns() + 1);
	column(Eigenmodes, 0) = averages;
	submatrix(Eigenmodes, 0, 1, W_decode.rows(), W_decode.columns()) = W_decode;
	return Eigenmodes;
}

// simple linear encoder based on PCA, accepts curves in rows

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const mtrc::numeric::DynamicMatrix<value_type> &TrainingData, size_t n_features)
{
	W_decode = mtrc::PCA(TrainingData, n_features, averages);
	W_encode = trans(W_decode); // computed once and saved
}

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const std::vector<RecType> &TrainingData, const size_t n_features)
{
	mtrc::numeric::DynamicMatrix<value_type> numeric_in(TrainingData.size(), TrainingData[0].size(), 0);
	for (size_t i = 0; i < TrainingData.size(); ++i)
		for (size_t j = 0; j < TrainingData[0].size(); ++j)
			numeric_in(i, j) = TrainingData[i][j];
	W_decode = mtrc::PCA(numeric_in, n_features, averages);
	W_encode = trans(W_decode); // computed once and saved
}

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const mtrc::numeric::DynamicMatrix<value_type> &Weights,
							const mtrc::numeric::DynamicVector<value_type, mtrc::numeric::rowVector> &avgs)
{
	W_decode = Weights;
	W_encode = trans(W_decode); // computed once and saved
	averages = avgs;
}

template <typename RecType, typename Metric>
PCFA<RecType, Metric>::PCFA(const std::vector<RecType> &Weights, const RecType &avgs)
{
	W_decode = mtrc::numeric::DynamicMatrix<value_type>(Weights.size(), Weights[0].size(), 0);
	for (size_t i = 0; i < Weights.size(); ++i)
		for (size_t j = 0; j < Weights[0].size(); ++j)
			W_decode(i, j) = Weights[i][j];
	W_encode = trans(W_decode); // computed once and saved
	averages = mtrc::numeric::DynamicVector<value_type, mtrc::numeric::rowVector>(avgs.size(), 0);
	for (size_t i = 0; i < avgs.size(); ++i) {
		averages[i] = avgs[i];
	}
}

template <typename RecType, typename Metric>
mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>
PCFA<RecType, Metric>::encode(const mtrc::numeric::DynamicMatrix<PCFA<RecType, Metric>::value_type> &Data)
{
	auto CenteredInput =
		mtrc::numeric::DynamicMatrix<PCFA<RecType, Metric>::value_type>(Data.rows(), Data.columns(), 0);
	for (size_t row_idx = 0; row_idx < Data.rows(); row_idx++)
		mtrc::numeric::row(CenteredInput, row_idx) = mtrc::numeric::row(Data, row_idx) - averages;
	return CenteredInput * W_encode;
}

template <typename RecType, typename Metric>
std::vector<RecType> PCFA<RecType, Metric>::encode(const std::vector<RecType> &Data)
{
	auto DataNumeric = records_to_matrix(Data);
	auto CenteredInput =
		mtrc::numeric::DynamicMatrix<PCFA<RecType, Metric>::value_type>(DataNumeric.rows(), DataNumeric.columns(), 0);
	for (size_t row_idx = 0; row_idx < DataNumeric.rows(); row_idx++)
		mtrc::numeric::row(CenteredInput, row_idx) = mtrc::numeric::row(DataNumeric, row_idx) - averages;
	mtrc::numeric::DynamicMatrix<PCFA<RecType, Metric>::value_type> Out = CenteredInput * W_encode;
	return matrix_to_records<RecType>(Out);
}

template <typename RecType, typename Metric>
mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>
PCFA<RecType, Metric>::decode(const mtrc::numeric::DynamicMatrix<PCFA<RecType, Metric>::value_type> &Codes,
							  bool unshift)
{
	if (unshift) {
		auto Noncentered = Codes * W_decode;
		auto Centered = mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(
			Noncentered.rows(), Noncentered.columns());
		for (size_t row_idx = 0; row_idx < Noncentered.rows(); row_idx++)
			mtrc::numeric::row(Centered, row_idx) = mtrc::numeric::row(Noncentered, row_idx) + averages;
		return Centered;
	} else {
		return Codes * W_decode;
	}
}

template <typename RecType, typename Metric>
std::vector<RecType> PCFA<RecType, Metric>::decode(const std::vector<RecType> &Codes, bool unshift)
{
	auto CodesNumeric = records_to_matrix(Codes);
	if (unshift) {
		auto Noncentered = CodesNumeric * W_decode;
		auto Centered = mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(
			Noncentered.rows(), Noncentered.columns());
		for (size_t row_idx = 0; row_idx < Noncentered.rows(); row_idx++)
			mtrc::numeric::row(Centered, row_idx) = mtrc::numeric::row(Noncentered, row_idx) + averages;
		return matrix_to_records<RecType>(Centered);
	} else {
		return matrix_to_records<RecType>(CodesNumeric * W_decode);
	}
}

template <typename RecType, typename Metric>
mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type> PCFA<RecType, Metric>::average_mat()
{
	auto avg = mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(1, averages.size());
	mtrc::numeric::row(avg, 0) = averages;
	return avg;
	// return expand(averages, 0);  // expand absents in local version of Numeric-lib
}

template <typename RecType, typename Metric> RecType PCFA<RecType, Metric>::average()
{
	mtrc::numeric::DynamicMatrix<value_type> result(1, averages.size());
	mtrc::numeric::row(result, 0) = averages;
	return matrix_to_records<RecType>(result)[0];
}

template <typename RecType, typename Metric> std::vector<RecType> PCFA<RecType, Metric>::weights()
{
	return matrix_to_records<RecType>(W_decode);
}

template <typename RecType, typename Metric>
mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type> PCFA<RecType, Metric>::eigenmodes_mat()
{
	auto Eigenmodes = mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>(W_decode.rows() + 1,
																								 W_decode.columns());
	mtrc::numeric::row(Eigenmodes, 0) = averages;
	submatrix(Eigenmodes, 1, 0, W_decode.rows(), W_decode.columns()) = W_decode;
	return Eigenmodes;
}

template <typename RecType, typename Metric> std::vector<RecType> PCFA<RecType, Metric>::eigenmodes()
{
	return matrix_to_records<RecType>(eigenmodes_mat());
}

template <typename RecType, typename Metric>
mtrc::numeric::DynamicMatrix<typename PCFA<RecType, Metric>::value_type>
PCFA<RecType, Metric>::records_to_matrix(const std::vector<RecType> &In)
{
	mtrc::numeric::DynamicMatrix<value_type> Out(In.size(), In[0].size(), 0);
	for (size_t i = 0; i < In.size(); ++i)
		for (size_t j = 0; j < In[0].size(); ++j)
			Out(i, j) = In[i][j];
	return Out;
}

template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if<determine_container_type<R>::code == 1,
						std::vector<R>>::type // here we support only STL vector
PCFA<RecType, Metric>::matrix_to_records(const mtrc::numeric::DynamicMatrix<typename PCFA<R, Metric>::value_type> &In)
{
	std::vector<RecType> Out;
	for (size_t i = 0; i < In.rows(); ++i) {
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
PCFA<RecType, Metric>::matrix_to_records(const mtrc::numeric::DynamicMatrix<typename PCFA<R, Metric>::value_type> &In)
{ // only numeric row-vector
	std::vector<RecType> Out;
	for (size_t i = 0; i < In.rows(); ++i) {
		RecType rec(In.columns()); // Metric numeric vector case
		for (size_t j = 0; j < In.columns(); ++j)
			rec[j] = In(i, j); // Metric numeric vector case
		Out.push_back(rec);
	}
	return Out;
}

template <typename Matrix>
PCFA_col<typename Matrix::ElementType> PCFA_col_factory(const Matrix &TrainingData, size_t n_features)
{
	return PCFA_col<typename Matrix::ElementType>(TrainingData, n_features);
}

template <typename ElementType>
PCFA<mtrc::numeric::DynamicMatrix<ElementType>, void>
PCFA_factory(mtrc::numeric::DynamicMatrix<ElementType> TrainingData,
			 size_t n_features) // special factory for Numeric matrix
{
	return PCFA<mtrc::numeric::DynamicMatrix<ElementType>, void>(TrainingData, n_features);
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

} // namespace mtrc
#endif
