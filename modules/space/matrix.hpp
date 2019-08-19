/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch

*/

#ifndef _METRIC_SPACE_MATRIX_HPP
#define _METRIC_SPACE_MATRIX_HPP

#include "../distance.hpp"

namespace metric {

/**
 * @class Matrix
 *
 * @brief distance matrix
 *
 */
template <typename recType, typename Metric = metric::Euclidian<typename recType::value_type>,
    typename distType = float>
class Matrix {
public:
    /*** Constructors ***/

    /**
     * @brief Construct a new empty Matrix
     *
     * @param d metric object to use as distance
     */
    Matrix(Metric d = Metric());

    /**
     * @brief Construct a new Matrix with one data record
     *
     * @param p data record
     * @param d metric object to use as distance
     */
    Matrix(const recType& p, Metric d = Metric());

    /**
     * @brief Construct a new Matrix with set of data records
     *
     * @param p vector of data records
     * @param d metric object to use as distance
     */

    Matrix(const std::vector<recType>& p, Metric d = Metric());

    /**
     * @brief Destroy the Matrix object
     *
     */
    ~Matrix() {};

    /*** Access Operations ***/

    /**
     * @brief append data record to the matrix
     *
     * @param p data record
     * @return true if append is successful
     * @return false if operation is unsuccesful
     */
    bool append(const recType& p);

    /**
     * @brief append data record into the Matrix only if distance bigger than a treshold
     *
     * @param p data record
     * @param treshold distance threshold
     * @return true if operation is successful
     * @return false if operation is unsuccesful
     */
    bool append_if(const recType& p, distType treshold);

    /**
     * @brief append set of data records to the matrix
     *
     * @param p vector of the new data records
     * @return true if append is successful
     * @return false if operation is unsuccesful
     */
    bool append(const std::vector<recType>& p);

    /**
     * @brief append data records into the Matrix only if distance bigger than a treshold
     *
     * @param p set of data records
     * @param treshold distance threshold
     * @return true if operation is successful
     * @return false if operation is unsuccesful
     */
    bool append_if(const std::vector<recType>& p, distType treshold);

    /**
     * @brief erase data record from Matrix by ID
     *
     * @param id ID of erased data record
     * @return true if operation successful
     * @return false if operation unsuccessful
     */
    bool erase(size_t id);

    /**
     * @brief change data record by ID
     *
     * @param id ID of data record
     * @param p  new data record
     * @return true if operation successful
     * @return false if operation unsucessful
     */
    bool set(size_t id, const recType& p);

    /**
     * @brief access a data record by ID
     *
     * @param id data record ID
     * @return data record with ID == id
     */
    recType operator[](size_t id) const;

    /**
     * @brief access a distance by two IDs
     *
     * @param i first ID
     * @param j second ID
     * @return distance between i and j
     */
    distType operator()(size_t i, size_t j) const;

    /*** information ***/

    /**
     * @brief size of matrix
     *
     * @return amount of data records
     */
    size_t size() const;

private:
    /*** Properties ***/
    Metric metric_;
    blaze::SymmetricMatrix<blaze::DynamicMatrix<distType>> D_;
    std::vector<recType> data_;
};

}  // namespace metric
#include "matrix.cpp"

#endif  // headerguard
