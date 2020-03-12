/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <boost/test/tools/old/interface.hpp>
#include <stdexcept>
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE space_matrix_test
#include <boost/test/unit_test.hpp>

#include "modules/space.hpp"

BOOST_AUTO_TEST_CASE(matrix_constructor) {
    // create empty matrix
    metric::Matrix<std::vector<float>, metric::Euclidian<float>> m;
    BOOST_TEST(m.size() == 0);

    // create matrix with initial values
    metric::Matrix<std::vector<float>, metric::Euclidian<float>> m1(std::vector<float>{1});
    BOOST_TEST(m1.size() == 1);

    std::vector<std::vector<float>> data = {{1}, {2}, {3}};
    metric::Matrix<std::vector<float>, metric::Euclidian<float>> m2(data);
    BOOST_TEST(m2.size() == 3);
}

BOOST_AUTO_TEST_CASE(matrix_insert) {
    metric::Matrix<float, metric::Euclidian<float>> m;
    BOOST_TEST(m.size() == 0);
    std::size_t id1 = m.insert(1.0);
    BOOST_TEST(id1 == 0);
    BOOST_TEST(m.size() == 1);

    std::vector<float> d = {1,2,3,4};
    auto ids = m.insert(d);
    BOOST_TEST(m.size() == 5);
    std::vector<std::size_t> ids_eta = {1, 2, 3, 4};
    BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), ids_eta.begin(),ids_eta.end());
}
