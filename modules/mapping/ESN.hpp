/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_ESN_HPP
#define _METRIC_MAPPING_ESN_HPP

#include "../utils.hpp"

#include "../../3rdparty/blaze/Blaze.h"



namespace metric {



template <typename RT>
class EmptyMetric {
public:
    using RecordType = RT;
    using ValueType = contained_value_t<RecordType>;

    template <typename R>
    auto operator()(const R&, const R&) -> DistanceType {
        return 0;
    }
};



/**
 * @class ESN
 * 
 * @brief 
 */
template <typename MT = EmptyMetric<std::vector<double>>>
class ESN {

public:

    using ValueType = typename MT::ValueType;
    using RecordType = typename MT::RecordType;

    /**
     * @brief Construct a new ESN object
     * 
     * @param w_size 
     * @param w_connections 
     * @param w_sr 
     * @param alpha_ 
     * @param washout_ 
     * @param beta_ 
     */
    ESN(
        size_t w_size = 500,  // number of elements in reservoir
        ValueType w_connections = 10,  // number of interconnections (for each reservoir element)
        ValueType w_sr = 0.6,  // desired spectral radius of the reservoir
        ValueType alpha_ = 0.5,  // leak rate
        size_t washout_ = 1,  // number of slices excluded from output for washout
        ValueType beta_ = 0.5  // ridge solver metaparameter
    );

    /**
     * @brief 
     * 
     * @param Slices 
     * @param Target 
     */
    void train(const blaze::DynamicMatrix<ValueType>& Slices, const blaze::DynamicMatrix<ValueType>& Target);

    /**
     * @brief
     *
     * @param Slices
     * @param Target
     */
    void train(const std::vector<RecordType> & Slices, const std::vector<RecordType> & Target);


    /**
     * @brief 
     * 
     * @param Slices 
     * @return 
     */
    blaze::DynamicMatrix<ValueType> predict(const blaze::DynamicMatrix<ValueType>& Slices);

    /**
     * @brief
     *
     * @param Slices
     * @return
     */
    std::vector<RecordType> predict(const std::vector<RecordType> & Slices);


private:
    blaze::DynamicMatrix<ValueType> W_in;
    blaze::CompressedMatrix<ValueType> W;
    blaze::DynamicMatrix<ValueType> W_out = blaze::DynamicMatrix<ValueType>(0, 0);
    bool trained = false;
    ValueType alpha = 0.5;
    ValueType beta = 0.5;
    size_t washout = 1;
    std::default_random_engine rgen;

    void create_W(size_t w_size, ValueType w_connections, ValueType w_sr);

    blaze::DynamicMatrix<ValueType> vector_to_blaze(const std::vector<RecordType> & In);

    template <typename R>
    typename std::enable_if <
     container_type<R>::code == 1,
     std::vector<R>
    >::type
    blaze2RecType(const blaze::DynamicMatrix<contained_value_t<R>> & In);

    template <typename R>
    typename std::enable_if<
     container_type<R>::code == 2,
     std::vector<R>
    >::type
    blaze2RecType(const blaze::DynamicMatrix<contained_value_t<R>> & In);

};  // class ESN

}  // namespace metric

#include "ESN.cpp"

#endif  // _METRIC_MAPPING_DETAILS_ESN_HPP
