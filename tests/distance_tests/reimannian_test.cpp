/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/


#include "../../modules/distance/d-spaced/Riemannian.hpp"

#define BOOST_TEST_DYN_LINK // added by Max F
#define BOOST_TEST_MODULE riemannian_test
#include <boost/test/unit_test.hpp>


#include <numeric>
#include <stdexcept>
//#include <limits> // only for cout precision

//typedef std::numeric_limits< double > dbl; // only for cout precision

template <typename T>
bool float_eq(T l, T r) {
    return std::abs(l -r) <= std::numeric_limits<T>::epsilon();
}


BOOST_AUTO_TEST_CASE(riemannian_distance) {

    blaze::DynamicMatrix<double> A {{1, 2, -3}, {0, -2, 4}, {0, 0, 1}};
    blaze::DynamicMatrix<double> B {{2, 0, -2}, {0, 1, 1}, {0, 0, 1}};

    auto rd = metric::RiemannianDistance<void, metric::Euclidean<double>>();
    //std::cout.precision(dbl::max_digits10);
    //std::cout << rd(A, B) << "\n";
    BOOST_TEST(float_eq(rd.matDistance(A, B), 0.6931471805599455));

    std::vector<std::vector<double>> ds1 {{0, 1}, {0, 0}, {1, 1}, {1, 0}};
    std::vector<std::vector<double>> ds2 {{0, 0}, {1, 1}, {2, 2}, {2, 1}};
    //std::cout.precision(dbl::max_digits10);
    //std::cout << rd(ds1, ds2) << "\n";
    BOOST_TEST(float_eq(rd(ds1, ds2), 0.8086438137089399));
}


