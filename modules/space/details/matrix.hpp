/* 
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch

*/

#ifndef _METRIC_SPACE_DETAILS_MATRIX_HPP
#define _METRIC_SPACE_DETAILS_MATRIX_HPP

#include "../../distance.hpp"

namespace metric {
namespace space {
    template <typename recType, typename Metric = metric::distance::Euclidian<typename recType::value_type>,
              typename distType = float>
class Matrix
{
  private:
    /*** Properties ***/
    Metric metric_;
    blaze::SymmetricMatrix<blaze::DynamicMatrix<distType>> D_;
    std::vector<recType> data_;

  public:
    /*** Constructors ***/
    Matrix(Metric d = Metric());                                // empty Matrix
    Matrix(const recType &p, Metric d = Metric());              // Matrix with one data record
    Matrix(const std::vector<recType> &p, Metric d = Metric()); // with a vector of data records
    ~Matrix();                                                  // destuctor

    /*** Access Operations ***/
    bool append(const recType &p);                              // append data record into the Matix
    bool append_if(const recType &p, distType treshold);        // append data record into the Matrix only if distance bigger than a treshold
    bool append(const std::vector<recType> &p);                 // append a vector of records into the Matrix
    bool append_if(const std::vector<recType> &p,distType treshold); // append a vector of records into the Matrix only if distance bigger than a treshold
    bool erase(size_t id);                                      // erase data record from Matrix by ID
    bool set(size_t id, const recType &p);                      // change data record by ID
    recType operator[](size_t id);                              // access a data record by ID
    distType operator()(size_t i, size_t j);                    // access a distance by two IDs
};

}
} // namespace metric
#include "matrix.cpp"

#endif // headerguard
