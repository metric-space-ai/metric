/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <boost/test/tools/old/interface.hpp>
#include <stdexcept>
//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE space_matrix_test
#include <boost/test/unit_test.hpp>

#include "modules/space.hpp"

namespace std {
std::basic_ostream<char> &operator<<(std::basic_ostream<char> &ostr, const std::pair<std::size_t, double> &v) {
    return ostr << "{" << v.first << ", " << v.second << "}";
}
}

BOOST_AUTO_TEST_CASE(matrix_constructor) {
    // create empty matrix
    metric::Matrix<std::vector<float>, metric::Euclidian<float>> m;
    BOOST_TEST(m.size() == 0);

    // create matrix with initial values
    metric::Matrix<std::vector<float>, metric::Euclidian<float>> m1(std::vector<float>{1});
    //    m1.print();
    BOOST_TEST(m1.size() == 1);

    std::vector<std::vector<float>> data = {{1}, {2}, {3}};
    metric::Matrix<std::vector<float>, metric::Euclidian<float>> m2(data);
    //    m2.print();
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
    BOOST_TEST(m(0, 1) == 0);
    BOOST_TEST(m(0, 2) == 1.0);
    BOOST_TEST(m(2, 0) == 1.0);
    BOOST_TEST(m(0, 3) == 2.0);
    BOOST_TEST(m(3, 0) == 2.0);
    BOOST_TEST(m.check_matrix());
}
BOOST_AUTO_TEST_CASE(matrix_insert_if) {
    metric::Matrix<float, metric::Euclidian<float>> m;
    BOOST_TEST(m.size() == 0);
    auto i1 = m.insert_if(1, 10);
    BOOST_TEST(m.size() == 1);
    BOOST_TEST(m[0] == 1);
    BOOST_TEST(i1.first  == 0);
    BOOST_TEST(i1.second == true);
    
    auto i2 = m.insert_if(2, 10);
    BOOST_TEST(m.size() == 1);
    BOOST_TEST(m[0] == 1);
    BOOST_TEST(i2 == std::pair(std::size_t(0),false));
    
    std::vector<float> d = {2,3,50, 13, 100};
    auto ids = m.insert_if(d,10);
    std::vector<std::pair<std::size_t, bool>> ids_eta = { {0, false}, {0, false}, {1, true}, {2, true}, {3, true} };
    BOOST_TEST(m.size() == 4);
    BOOST_TEST(m[0] == 1);
    BOOST_TEST(m[1] == 50);
    BOOST_TEST(m[2] == 13);
    BOOST_TEST(m[3] == 100);
    BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), ids_eta.begin(), ids_eta.end());
    BOOST_TEST(m.check_matrix());
}

BOOST_AUTO_TEST_CASE(matrix_set) {
    std::vector<float> data(10);
    std::iota(data.begin(), data.end(), 0);
    metric::Matrix<float, metric::Euclidian<float>> m(data);
    BOOST_TEST(m.size() == 10);
    BOOST_TEST(m.check_matrix());

    m.set(0,100);
    BOOST_TEST(m[0] == 100);
    BOOST_TEST(m.check_matrix());

    BOOST_TEST(m[1] == 1);
    m.set(1,10);
    BOOST_TEST(m[1] == 10);
    BOOST_TEST(m.check_matrix());
}
BOOST_AUTO_TEST_CASE(matrix_erase) {
    std::vector<float> data(10);
    std::iota(data.begin(), data.end(), 0);

    metric::Matrix<float, metric::Euclidian<float>> m(data);

    BOOST_TEST(m.check_matrix());
    BOOST_TEST(m.size() == 10);
    m.erase(0);

    BOOST_TEST(m.size() == 9);
    auto nn = m.nn(1);
    BOOST_TEST(nn == 0);
    BOOST_TEST(m[nn] == 1);
    BOOST_TEST(m.check_matrix());

    auto item2 = m[2];
    m.erase(2);
    BOOST_TEST(m.size() == 8);
    auto nn1 = m.nn(item2);
    BOOST_TEST(nn1 != item2);
    BOOST_TEST(m[0] == 1);
    BOOST_TEST(m[1] == 2);
    BOOST_TEST(m.check_matrix());
    BOOST_CHECK_THROW(m.erase(11), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_knn) {
    metric::Matrix<double, metric::Euclidian<double>> m;
    for(int i = 0; i < 10; i++) {
        m.insert(static_cast<double>(i));
    }
    auto knn1 = m.knn(0, 3);
    BOOST_TEST(knn1.size() == std::size_t(3));
    using knn_type_t = std::vector<std::pair<std::size_t, double>>;
    knn_type_t e1{{0, 0}, {1, 1}, {2, 2}};
    BOOST_CHECK_EQUAL_COLLECTIONS(knn1.begin(), knn1.end(), e1.begin(), e1.end());

    auto knn2 = m.knn(0, 11);
    BOOST_TEST(knn2.size() == std::size_t(10));

    knn_type_t e2{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9,9}};
    BOOST_CHECK_EQUAL_COLLECTIONS(knn2.begin(), knn2.end(), e2.begin(), e2.end());

    auto knn3 = m.knn(1,3);
    BOOST_TEST(knn3.size() == std::size_t(3));
    knn_type_t e3{{1, 0}, {0, 1}, {2, 1}};
    BOOST_CHECK_EQUAL_COLLECTIONS(knn3.begin(), knn3.end(), e3.begin(), e3.end());

    auto knn4 = m.knn(0.5, 3);
    BOOST_TEST(knn4.size() == std::size_t(3));
    knn_type_t e4{{0, 0.5}, {1, 0.5}, {2, 1.5}};
    BOOST_CHECK_EQUAL_COLLECTIONS(knn4.begin(), knn4.end(), e4.begin(), e4.end());
}

BOOST_AUTO_TEST_CASE(test_rnn) {
    metric::Matrix<double, metric::Euclidian<double>> m{};
    
    for (int i = 0; i < 10; i++) {
        m.insert(static_cast<double>(i));
    }
    auto knn1 = m.rnn(0, 2.0);
    BOOST_TEST(knn1.size() == std::size_t(3));
    using knn_type_t = std::vector<std::pair<std::size_t, double>>;
    knn_type_t e1{{0, 0}, {1, 1}, {2, 2}};
    BOOST_CHECK_EQUAL_COLLECTIONS(knn1.begin(), knn1.end(), e1.begin(), e1.end());

    auto knn2 = m.rnn(0, 12.0);
    BOOST_TEST(knn2.size() == std::size_t(10));
    knn_type_t e2{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}};
    BOOST_CHECK_EQUAL_COLLECTIONS(knn2.begin(), knn2.end(), e2.begin(), e2.end());
}
