/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_PCFA_HPP
#define _METRIC_MAPPING_PCFA_HPP

#include <blaze/Blaze.h>
#include <type_traits>

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
blaze::DynamicMatrix<typename BlazeMatrix::ElementType>
PCA_col(const BlazeMatrix &In, int n_components, blaze::DynamicVector<typename BlazeMatrix::ElementType> &averages);

/**
 * @brief
 *
 * @param In
 * @param n_components
 * @param averages - outputs average curve
 * @return
 */
template <class BlazeMatrix>
blaze::DynamicMatrix<typename BlazeMatrix::ElementType>
PCA(const BlazeMatrix &In, int n_components,
	blaze::DynamicVector<typename BlazeMatrix::ElementType, blaze::rowVector> &averages);

/**
 * @class PCFA_col
 *
 *@brief simple linear encoder based on PCA for data organized by columns
 *
 */
template <typename V> class PCFA_col {

  public:
	using value_type = V;

	/**
	 * @brief Construct a new PCFA_col object
	 *
	 * @param TrainingData - training dataset with curves in columns
	 * @param n_features - desired length of compressed code
	 */
	PCFA_col(const blaze::DynamicMatrix<value_type> &TrainingData, size_t n_features = 1);

	/**
	 * @brief
	 *
	 * @param Data
	 * @return
	 */
	blaze::DynamicMatrix<value_type> encode(const blaze::DynamicMatrix<value_type> &Data);

	/**
	 * @brief
	 *
	 * @param Codes
	 * @param unshift - flag for adding average curve to each decoded one
	 * @return blaze::DynamicMatrix<value_type>
	 */
	blaze::DynamicMatrix<value_type> decode(const blaze::DynamicMatrix<value_type> &Codes, bool unshift = true);

	/**
	 * @brief returns the average curve of training dataset, used for center shift
	 *
	 * @return blaze::DynamicMatrix<value_type>
	 */
	blaze::DynamicMatrix<value_type> average();

	/**
	 * @brief returns the encoder matrix concatenated with the average curve of training dataset, used for center shift
	 *
	 * @return blaze::DynamicMatrix<value_type>
	 */
	blaze::DynamicMatrix<value_type> eigenmodes();

  private:
	blaze::DynamicMatrix<value_type> W_decode;
	blaze::DynamicMatrix<value_type> W_encode;
	blaze::DynamicVector<value_type> averages;
	std::default_random_engine rgen;
};

// common metafunctions for PCFA and DSPCC

template <typename>
struct determine_container_type // checks whether container is STL container (1) or Blaze vector (2)
{
	constexpr static int code = 0;
};

template <template <typename, typename> class Container, typename ValueType, typename Allocator>
struct determine_container_type<Container<ValueType, Allocator>> {
	constexpr static int code = 1;
};

template <template <typename, bool> class Container, typename ValueType, bool F>
struct determine_container_type<Container<ValueType, F>> {
	constexpr static int code = 2;
};

//    template<typename>
//    struct determine_element_type  // old version, replaced with value_type/ElementType field detector!
//    {
//        using type = void;
//    };

//    template <typename ValueType, typename Allocator>
//    struct determine_element_type<std::vector<ValueType, Allocator>>
//    {
//        using type = typename std::vector<ValueType, Allocator>::value_type;
//    };

//    template <typename ValueType, bool F>
//    struct determine_element_type<blaze::DynamicVector<ValueType, F>>
//    {
//        using type = typename blaze::DynamicVector<ValueType, F>::ElementType;
//    };

//    template <typename ValueType, bool F>
//    struct determine_element_type<blaze::DynamicMatrix<ValueType, F>>
//    {
//        using type = typename blaze::DynamicMatrix<ValueType, F>::ElementType;
//    };

template <typename C, int = determine_container_type<C>::code>
struct determine_element_type // determines type of element both for STL containers and Blaze vectors
{
	using type = void;
};

template <typename C> struct determine_element_type<C, 1> {
	using type = typename C::value_type;
};

template <typename C> struct determine_element_type<C, 2> {
	using type = typename C::ElementType;
};

/**
 * @class PCFA
 *
 *@brief simple linear encoder based on PCA
 *
 */
template <typename RecType, typename Metric> class PCFA {

	// private:

  public:
	using value_type = typename determine_element_type<RecType>::type;

	// constexpr static int ContainerCode = determine_container_type<RecType>::code;

	/**
	 * @brief Construct a new PCFA object from dataset in blaze DynamicMatrix
	 *
	 * @param TrainingData - training dataset with curves in rows
	 * @param n_features - desired length of compressed code
	 */
	PCFA(const blaze::DynamicMatrix<value_type> &TrainingData, size_t n_features = 1);

	/**
	 * @brief Construct a new PCFA object vrom vector of records
	 *
	 * @param TrainingData - training dataset, vector of records
	 * @param n_features - desired length of compressed code
	 */
    PCFA(const std::vector<RecType> &TrainingData, const size_t n_features = 1);

    /**
     * @brief Construct PCFA from trained decode weight matrix and vector of averages
     * @param Weights
     * @param averages
     */
    PCFA(const blaze::DynamicMatrix<value_type> &Weights,
         const blaze::DynamicVector<value_type, blaze::rowVector> &avgs);

    /**
     * @brief Construct PCFA from trained decode weight matrix and vector of averages given as vector containers
     * @param Weights
     * @param avgs
     */
    PCFA(const std::vector<RecType> &Weights, const RecType &avgs);

	/**
	 * @brief
	 *
	 * @param Data
	 * @return
	 */
	blaze::DynamicMatrix<value_type> encode(const blaze::DynamicMatrix<value_type> &Data);

	/**
	 * @brief
	 *
	 * @param Data
	 * @return
	 */
	std::vector<RecType> encode(const std::vector<RecType> &Data);

	/**
	 * @brief
	 *
	 * @param Codes
	 * @param unshift - flag for adding average curve to each decoded one
	 * @return blaze::DynamicMatrix<value_type>
	 */
	blaze::DynamicMatrix<value_type> decode(const blaze::DynamicMatrix<value_type> &Codes, bool unshift = true);

	/**
	 * @brief
	 *
	 * @param Data
	 * @return
	 */
	std::vector<RecType> decode(const std::vector<RecType> &Data, bool unshift = true);

	/**
	 * @brief returns the average curve of training dataset, used for center shift
	 *
	 * @return blaze::DynamicMatrix<value_type>
	 */
	blaze::DynamicMatrix<value_type> average_mat();

	/**
	 * @brief returns the average curve of training dataset, used for center shift
	 *
     * @return RecType
	 */
    // std::vector<RecType> average();
    RecType average();

    /**
     * @brief returns weights
     *
     * @return matrix of decode weights
     */
    std::vector<RecType> weights();

	/**
	 * @brief returns the encoder matrix concatenated with the average curve of training dataset, used for center shift
	 *
	 * @return blaze::DynamicMatrix<value_type>
	 */
	blaze::DynamicMatrix<value_type> eigenmodes_mat();

	/**
	 * @brief returns the encoder matrix concatenated with the average curve of training dataset, used for center shift
	 *
	 * @return blaze::DynamicMatrix<value_type>
	 */
	std::vector<RecType> eigenmodes();

  private:
	blaze::DynamicMatrix<value_type> W_decode;
	blaze::DynamicMatrix<value_type> W_encode;
	blaze::DynamicVector<value_type, blaze::rowVector> averages;
	std::default_random_engine rgen;

	blaze::DynamicMatrix<value_type> vector_to_blaze(const std::vector<RecType> &In);

	template <typename R>
	typename std::enable_if<determine_container_type<R>::code == 1, std::vector<R>>::type
	blaze2RecType(const blaze::DynamicMatrix<typename PCFA<R, Metric>::value_type> &In);

	template <typename R>
	typename std::enable_if<determine_container_type<R>::code == 2, std::vector<R>>::type
	blaze2RecType(const blaze::DynamicMatrix<typename PCFA<R, Metric>::value_type> &In);
};

/**
 * @brief Creates a new PCFA_col object from dataset of Blaze Matrix type
 *
 * @param TrainingData
 * @param n_features
 */
template <typename BlazeMatrix>
PCFA_col<typename BlazeMatrix::ElementType> PCFA_col_factory(const BlazeMatrix &TrainingData, size_t n_features = 1);

/**
 * @brief Creates a new PCFA object from dataset of Blaze Matrix type
 *
 * @param TrainingData
 * @param n_features
 */
template <typename BlazeMatrix>
PCFA<typename BlazeMatrix::ElementType, void> PCFA_factory(const BlazeMatrix &TrainingData, size_t n_features = 1);

/**
 * @brief Creates a new PCFA object from vector of STL containers as dataset
 *
 * @param TrainingData
 * @param n_features
 */
template <template <typename, typename> class Container, typename ValueType, typename Allocator>
PCFA<Container<ValueType, Allocator>, void> PCFA_factory(std::vector<Container<ValueType, Allocator>> &TrainingData,
														 size_t n_features = 1);

/**
 * @brief Creates a new PCFA object from vector of Blaze vectors as dataset
 *
 * @param TrainingData
 * @param n_features
 */
template <template <typename, bool> class Container, typename ValueType, bool F>
PCFA<Container<ValueType, F>, void> PCFA_factory(std::vector<Container<ValueType, F>> &TrainingData,
												 size_t n_features = 1);

} // namespace metric

#include "PCFA.cpp"

#endif // _METRIC_MAPPING_PCFA_HPP
