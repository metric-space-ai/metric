/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch

*/

#ifndef _METRIC_SPACE_MATRIX_HPP
#define _METRIC_SPACE_MATRIX_HPP

#include <blaze/Blaze.h>

#include <type_traits>
#include <unordered_map>
#include <vector>
#include <iostream>

namespace metric {

/**
 * @class Matrix
 *
 * @brief distance matrix
 *
 */
template <typename RecType, typename Metric>
class Matrix {
public:
    using distType = typename std::invoke_result<Metric, const RecType&, const RecType&>::type;
    
    /*** Constructors ***/

    /**
     * @brief Construct a new empty Matrix
     *
     * @param d metric object to use as distance
     */
    explicit Matrix(Metric d = Metric())
        : metric_(d)
    {
    }

    /**
     * @brief Construct a new Matrix with one data record
     *
     * @param p data record
     * @param d metric object to use as distance
     */
    explicit Matrix(const RecType& p, Metric d = Metric())
        : metric_(d)
    {
        insert(p);
    }

    /**
     * @brief Construct a new Matrix with set of data records
     *
     * @param p random access container of data records
     * @param d metric object to use as distance
     */
    template <typename Container,
              typename =  std::enable_if<
                  std::is_same<RecType, typename std::decay<decltype(std::declval<Container>().operator[](0))>::type>::value>>
    explicit Matrix(const Container& p, Metric d = Metric())
        : metric_(d)
    {
        insert(p);
    }

    /*
     * Set of default methods
     *
     */
    ~Matrix() = default;
    Matrix(const Matrix &) = default;
    Matrix(Matrix&&) noexcept = default;
    auto operator = (Matrix&&) noexcept -> Matrix& = default;
    auto operator = (const Matrix&) -> Matrix& = default;

    /**
     * @brief append data record to the matrix
     *
     * @param p data record
     * @return Index of inserted node
     */
    auto insert(const RecType& item) -> std::size_t;

    /**
     * @brief append data record into the Matrix only if distance bigger than a treshold
     *
     * @param p data record
     * @param treshold distance threshold
     * @return pair consist of index and result of insertion
     */
    auto insert_if(const RecType& item, distType treshold) ->   std::pair<std::size_t, bool>;

        /**
     * @brief append set of data records to the matrix
     *
     * @param p random access container with new data records
     * @return vector of indexes of inserted node
     */
    template <typename Container,
              typename = std::enable_if<
                  std::is_same<RecType,
                               typename std::decay<decltype(std::declval<Container>().operator[](0))>::type                         >::value>>
    auto insert(const Container& items) -> std::vector<std::size_t>;

    /**
     * @brief append data records into the Matrix only if distance bigger than a treshold
     *
     * @param p set of data records
     * @param treshold distance threshold
     * @return vector of pairs consists of ID and result of insertion
     */
    template <typename Container,
              typename = std::enable_if<
                  std::is_same<RecType,
                               typename std::decay<decltype(std::declval<Container>().operator[](0))>::type
                               >::value>>
    auto insert_if(const Container& items, distType treshold) -> std::vector<std::pair<std::size_t, bool>>;

    /**
     * @brief erase data record from Matrix by ID
     *
     * @param index Index of erased data record
     * @return true if operation successful
     * @return false if operation unsuccessful
     */
    auto erase(size_t index) -> bool;

    /**
     * @brief change data record by ID
     *
     * @param index Index of data record
     * @param p  new data record
     */
    void set(size_t index, const RecType& p);

    /**
     * @brief access a data record by ID
     *
     * @param index data record index
     * @return data record at index
     * @throws std::runtime_error when index >= size()
     */
    auto operator[](size_t index) const -> RecType;
    

    /**
     * @brief access a distance by two IDs
     *
     * @param i row index
     * @param j column index
     * @return distance between i and j
     */
    auto  operator()(size_t i, size_t j) const -> distType;

    /*** information ***/

    /**
     * @brief size of matrix
     *
     * @return amount of data records
     */
    auto size() const -> std::size_t;

    /**
     * @brief find nearest neighbour of data record
     *
     * @param p searching data record
     * @return ID of nearest neighbour to p
     */
    auto nn(const RecType& p) const -> std::size_t;

    /**
     * @brief find  K nearest neighbours of data record
     *
     * @param query searching data record
     * @param k amount of nearest neighbours
     * @return vector of pair of node ID and distance to searching point
     */
    auto knn(const RecType& query, unsigned k = 10) const -> std::vector<std::pair<std::size_t, distType>>;
    
    /**
     * @brief find all nearest neighbour in range [0;distance]
     *
     * @param query searching point
     * @param range max distance to searching point
     * @return vector of pair of node ID and distance to searching point
     */

    auto rnn(const RecType& query, distType range = 1.0) const -> std::vector<std::pair<std::size_t, distType>>;

    /**
     * @brief debug function, check consistence of distance matrix
     *
     * @return  true if matrix is OK, false otherwise.
     */
    auto check_matrix() const -> bool {
        for(std::size_t i = 0; i < data_.size(); i++) {
            for(std::size_t j = 0; j < data_.size(); j++) {
                auto m = metric_(data_[i], data_[j]);
                auto d = (*this)(i, j);
                if (std::abs(m-d) >= std::numeric_limits<distType>::epsilon())  {
                    std::cout << "Check matrix failed at [" << i << "," << j
                              <<"] = dist(" << data_[i] <<" ," << data_[j] <<  ") : "
                              << m << " != " << d << std::endl; 
                    return false;
                }
            }
        }
        return true;
    }
    /**
     * @brief debug function, print distance matrix to stdout
     *
     */
    void print() const
    {
        std::cout << "D_=\n" << D_ << std::endl;
        std::cout << "non_zeros=" << D_.nonZeros() << std::endl;
    }

private:
    auto nn_(const RecType& p) const -> std::pair<std::size_t, distType>;
    auto nn_(const RecType& p, std::unordered_map<std::size_t, distType> & metric_cache) const -> std::pair<std::size_t, distType>;

    void check_index(std::size_t index) const {
        if(index >= data_.size()) {
            throw std::invalid_argument(std::string("no element with such ID:") + std::to_string(index) + " >= " + std::to_string(data_.size()));
        }
    }
    void remove_data(std::size_t index)
    {
        auto p = data_.begin();
        std::advance(p, index);
        data_.erase(p);
    }

    /*** Properties ***/
    Metric metric_;
    blaze::CompressedMatrix<distType> D_;
    std::vector<RecType> data_;
    mutable std::unordered_map<std::size_t, std::size_t> index_map_;
    std::vector<std::size_t> id_map_;
};

}  // namespace metric
#include "matrix.cpp"

#endif  // headerguard
