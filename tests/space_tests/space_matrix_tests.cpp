/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <catch2/catch.hpp>

#include <stdexcept>

#include "metric/metric.hpp"


TEMPLATE_TEST_CASE("matrix_constructor", "[space]", float, double) {
    // create empty matrix
    metric::Matrix<std::vector<TestType>, metric::Euclidean<TestType>> m;
    REQUIRE(m.size() == std::size_t(0));

    // create matrix with initial values
    metric::Matrix<std::vector<TestType>, metric::Euclidean<TestType>> m1(std::vector<TestType>{1});
    REQUIRE(m1.size() == std::size_t(1));

    std::vector<std::vector<TestType>> data = {{1}, {2}, {3}};
    metric::Matrix<std::vector<TestType>, metric::Euclidean<TestType>> m2(data);
    REQUIRE(m2.size() == std::size_t(3));
}

TEMPLATE_TEST_CASE("matrix_insert", "[space]", float, double) {
    metric::Matrix<TestType, metric::Euclidean<TestType>> m;
    REQUIRE(m.size() == std::size_t(0));
    std::size_t id1 = m.insert(TestType(1.0));
    REQUIRE(id1 == std::size_t(0));
    REQUIRE(m.size() == std::size_t(1));

    std::vector<TestType> d = {1,2,3,4};
    auto ids = m.insert(d);
    REQUIRE(m.size() == std::size_t(5));
    std::vector<std::size_t> ids_eta = {1, 2, 3, 4};
    //BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), ids_eta.begin(),ids_eta.end());
    REQUIRE(ids == ids_eta);
    REQUIRE(m(0, 1) == TestType(0));
    REQUIRE(m(0, 2) == TestType(1.0));
    REQUIRE(m(2, 0) == TestType(1.0));
    REQUIRE(m(0, 3) == TestType(2.0));
    REQUIRE(m(3, 0) == TestType(2.0));
    REQUIRE(m.check_matrix());
}

TEMPLATE_TEST_CASE("matrix_insert_if", "[space]", float, double) {
    metric::Matrix<TestType, metric::Euclidean<TestType>> m;
    REQUIRE(m.size() == std::size_t(0));
    auto i1 = m.insert_if(TestType(1), TestType(10));
    REQUIRE(m.size() == std::size_t(1));
    REQUIRE(m[0] == TestType(1));
    REQUIRE(i1.first  == bool(0));
    REQUIRE(i1.second == true);
    
    auto i2 = m.insert_if(TestType(2), TestType(10));
    REQUIRE(m.size() == std::size_t(1));
    REQUIRE(m[0] == TestType(1));
    REQUIRE(i2 == std::pair(std::size_t(0),false));
    
    std::vector<TestType> d = {2,3,50, 13, 100};
    auto ids = m.insert_if(d,10);
    std::vector<std::pair<std::size_t, bool>> ids_eta = { {0, false}, {0, false}, {1, true}, {2, true}, {3, true} };
    REQUIRE(m.size() == 4);
    REQUIRE(m[0] == TestType(1));
    REQUIRE(m[1] == TestType(50));
    REQUIRE(m[2] == TestType(13));
    REQUIRE(m[3] == TestType(100));
    //BOOST_CHECK_EQUAL_COLLECTIONS(ids.begin(), ids.end(), ids_eta.begin(), ids_eta.end());
    REQUIRE(ids == ids_eta);
    REQUIRE(m.check_matrix());
}

TEMPLATE_TEST_CASE("matrix_set", "[space]", float, double) {
    std::vector<TestType> data(10);
    std::iota(data.begin(), data.end(), 0);
    metric::Matrix<TestType, metric::Euclidean<TestType>> m(data);
    REQUIRE(m.size() == std::size_t(10));
    REQUIRE(m.check_matrix());

    m.set(std::size_t(0),TestType(100));
    REQUIRE(m[0] == TestType(100));
    REQUIRE(m.check_matrix());

    REQUIRE(m[1] == TestType(1));
    m.set(std::size_t(1),TestType(10));
    REQUIRE(m[1] == TestType(10));
    REQUIRE(m.check_matrix());
}
TEMPLATE_TEST_CASE("matrix_erase", "[space]", float, double) {
    std::vector<TestType> data(10);
    std::iota(data.begin(), data.end(), 0);

    metric::Matrix<TestType, metric::Euclidean<TestType>> m(data);

    REQUIRE(m.check_matrix());
    REQUIRE(m.size() == std::size_t(10));
    m.erase(0);

    REQUIRE(m.size() == std::size_t(9));
    auto nn = m.nn(TestType(1));
    REQUIRE(nn == std::size_t(0));
    REQUIRE(m[nn] == TestType(1));
    REQUIRE(m.check_matrix());

    auto item2 = m[2];
    m.erase(2);
    REQUIRE(m.size() == std::size_t(8));
    auto nn1 = m.nn(item2);
    REQUIRE(nn1 != item2);
    REQUIRE(m[0] == TestType(1));
    REQUIRE(m[1] == TestType(2));
    REQUIRE(m.check_matrix());
    //BOOST_CHECK_THROW(m.erase(11), std::invalid_argument);
    REQUIRE_THROWS_AS(m.erase(11), std::invalid_argument);
}

TEMPLATE_TEST_CASE("test_knn", "[space]", float, double) {
    metric::Matrix<TestType, metric::Euclidean<TestType>> m;
    for(int i = 0; i < 10; i++) {
        m.insert(static_cast<TestType>(i));
    }
    auto knn1 = m.knn(0.0, std::size_t(3));
    REQUIRE(knn1.size() == std::size_t(3));
    using knn_type_t = std::vector<std::pair<std::size_t, TestType>>;
    knn_type_t e1{{0, 0}, {1, 1}, {2, 2}};
    //BOOST_CHECK_EQUAL_COLLECTIONS(knn1.begin(), knn1.end(), e1.begin(), e1.end());
    REQUIRE(knn1 == e1);

    auto knn2 = m.knn(0.0, std::size_t(11));
    REQUIRE(knn2.size() == std::size_t(10));

    knn_type_t e2{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9,9}};
    //BOOST_CHECK_EQUAL_COLLECTIONS(knn2.begin(), knn2.end(), e2.begin(), e2.end());
	REQUIRE(knn2 == e2);

	auto knn3 = m.knn(1.0,std::size_t(3));
    REQUIRE(knn3.size() == std::size_t(3));
    knn_type_t e3{{1, 0}, {0, 1}, {2, 1}};
    //BOOST_CHECK_EQUAL_COLLECTIONS(knn3.begin(), knn3.end(), e3.begin(), e3.end());
	REQUIRE(knn3 == e3);

    auto knn4 = m.knn(0.5, std::size_t(3));
    REQUIRE(knn4.size() == std::size_t(3));
    knn_type_t e4{{0, 0.5}, {1, 0.5}, {2, 1.5}};
    //BOOST_CHECK_EQUAL_COLLECTIONS(knn4.begin(), knn4.end(), e4.begin(), e4.end());
	REQUIRE(knn4 == e4);
}

TEMPLATE_TEST_CASE("test_rnn", "[space]", float, double) {
    metric::Matrix<TestType, metric::Euclidean<TestType>> m{};
    
    for (int i = 0; i < 10; i++) {
        m.insert(static_cast<TestType>(i));
    }
    auto knn1 = m.rnn(0.0, 2.0);
    REQUIRE(knn1.size() == std::size_t(3));
    using knn_type_t = std::vector<std::pair<std::size_t, TestType>>;
    knn_type_t e1{{0, 0}, {1, 1}, {2, 2}};
    //BOOST_CHECK_EQUAL_COLLECTIONS(knn1.begin(), knn1.end(), e1.begin(), e1.end());
	REQUIRE(knn1 == e1);

    auto knn2 = m.rnn(0.0, 12.0);
    REQUIRE(knn2.size() == std::size_t(10));
    knn_type_t e2{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}};
    //BOOST_CHECK_EQUAL_COLLECTIONS(knn2.begin(), knn2.end(), e2.begin(), e2.end());
	REQUIRE(knn2 == e2);
}