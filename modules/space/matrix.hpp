/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch

*/

#ifndef _METRIC_SPACE_MATRIX_HPP
#define _METRIC_SPACE_MATRIX_HPP

#include "../distance.hpp"
#include <type_traits>
namespace metric {

/**
 * @class Matrix
 *
 * @brief distance matrix
 *
 */
template <typename recType, typename Metric>
class Matrix {
public:
    using distType = typename Metric::distance_type;
    
    /*** Constructors ***/

    /**
     * @brief Construct a new empty Matrix
     *
     * @param d metric object to use as distance
     */
    explicit Matrix(Metric d = Metric()):metric_(d) {}

    /**
     * @brief Construct a new Matrix with one data record
     *
     * @param p data record
     * @param d metric object to use as distance
     */
    explicit Matrix(const recType& p, Metric d = Metric()): metric_(d) {
        insert(p);
    }

    /**
     * @brief Construct a new Matrix with set of data records
     *
     * @param p vector of data records
     * @param d metric object to use as distance
     */
    template <typename Container,
              typename =  std::enable_if<
                  std::is_same<recType, typename std::decay<decltype(std::declval<Container>().operator[](0))>::type>::value>>
    explicit Matrix(const Container& p, Metric d = Metric())
        : metric_(d)
    {
        insert(p);
    }

    /**
     * @brief Destroy the Matrix object
     *
     */
    ~Matrix() {};

    /**
     * @brief append data record to the matrix
     *
     * @param p data record
     * @return ID of inserted node
     */
    std::size_t insert(const recType& p);

    /**
     * @brief append data record into the Matrix only if distance bigger than a treshold
     *
     * @param p data record
     * @param treshold distance threshold
     * @return pair consist of ID and result of insertion
     */
    std::pair<std::size_t, bool> insert_if(const recType& p, distType treshold);

    /**
     * @brief append set of data records to the matrix
     *
     * @param p vector of the new data records
     * @return vector of ID of inserted node
     * @return false if operation is unsuccesful
     */
    template <typename Container,
        typename = std::enable_if<
            std::is_same<recType, typename std::decay<decltype(std::declval<Container>().operator[](0))>::type>
            ::value>>
    std::vector<std::size_t> insert(const Container& p);

    /**
     * @brief append data records into the Matrix only if distance bigger than a treshold
     *
     * @param p set of data records
     * @param treshold distance threshold
     * @return vector of pairs consists of ID and result of insertion
     */
    template<typename Container>
    std::vector<std::pair<std::size_t, bool>> insert_if(const Container & p, distType treshold);

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
     */
    void set(size_t id, const recType& p);

    /**
     * @brief access a data record by ID
     *
     * @param id data record ID
     * @return data record with ID == id
     * @throws std::runtime_error when matrix has no element with ID
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

    /**
     * @brief find nearest neighbour of data record
     *
     * @param p searching data record
     * @return ID of nearest neighbour to p
     */
    std::size_t nn(const recType & p) const;

    /**
     * @brief find  K nearest neighbours of data record
     *
     * @param p searching data record
     * @param k amount of nearest neighbours
     * @return vector of pair of node ID and distance to searching point
     */
    std::vector<std::pair<std::size_t, distType>> knn(const recType & p, unsigned k = 10) const;

    /**
     * @brief find all nearest neighbour in range [0;distance]
     *
     * @param p searching point
     * @param distance max distance to searching point
     * @return vector of pair of node ID and distance to searching point
     */

    std::vector<std::pair<std::size_t, distType>> rnn(const recType & p, distType distance = 1.0) const;

private:
    /*** Properties ***/
    Metric metric_;
    blaze::SymmetricMatrix<blaze::DynamicMatrix<distType>> D_;
    std::vector<recType> data_;
};

}  // namespace metric
#include "matrix.cpp"

#endif  // headerguard
