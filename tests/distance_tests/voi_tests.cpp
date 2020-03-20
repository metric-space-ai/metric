/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include "modules/utils/type_traits.hpp"
#include <stdexcept>
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE voi_test
#include <boost/test/unit_test.hpp>

#include "modules/distance/k-random/VOI.hpp"
#include "modules/distance/k-structured/Edit.hpp"
#include "modules/distance/k-related/Standards.hpp"
#include <vector>
#include <deque>
#include <array>
#include <numeric>

template <typename T>
bool float_eq(T l, T r) {
    return std::abs(l -r) <= std::numeric_limits<T>::epsilon();
}
BOOST_AUTO_TEST_CASE(entropy) {
    std::vector<std::vector<double>>  v1 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::vector<std::deque<double>>   v2 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::vector<std::array<double,2>> v3 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::deque<std::vector<double>>   v4 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::deque<std::array<double, 2>> v7 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    BOOST_TEST(float_eq(metric::entropy(v1, 3, 2.0, metric::Chebyshev<double>()), 7.625814583693912));
    BOOST_TEST(float_eq(metric::entropy(v2, 3, 2.0, metric::Chebyshev<double>()), 7.625814583693912));
    BOOST_TEST(float_eq(metric::entropy(v3, 3, 2.0, metric::Chebyshev<double>()), 7.625814583693912));
    BOOST_TEST(float_eq(metric::entropy(v4, 3, 2.0, metric::Chebyshev<double>()), 7.625814583693912));
    // BOOST_TEST(metric::entropy(v5, 3, 2.0, metric::Chebyshev<double>()) == 7.625814583693912);
    // BOOST_TEST(metric::entropy(v6, 3, 2.0, metric::Chebyshev<double>()) == 7.625814583693912);
    BOOST_TEST(float_eq(metric::entropy(v7, 3, 2.0, metric::Chebyshev<double>()), 7.625814583693912));

    BOOST_TEST(float_eq(metric::entropy(v1),7.8197919635268081 ));
    BOOST_TEST(float_eq(metric::entropy(v2), 7.8197919635268081));
    BOOST_TEST(float_eq(metric::entropy(v3), 7.8197919635268081));
    BOOST_TEST(float_eq(metric::entropy(v4), 7.8197919635268081));
    BOOST_TEST(float_eq(metric::entropy(v7), 7.8197919635268081));

    std::vector<std::string> v8 = { "AAA", "HJGJHFG", "BBB", "AAAA", "long long long long long long string" };
    BOOST_TEST(float_eq(metric::entropy(v8, 3, 2.0, metric::Edit<int>()), 13.837977646415816));
}

BOOST_AUTO_TEST_CASE(mutual_information_float) {

    std::vector<std::vector<double>> v11 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::vector<double>> v12 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::mutualInformation(v11, v12);

    std::vector<std::deque<double>> v21 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::deque<double>> v22 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::mutualInformation(v21, v22);

    std::deque<std::vector<double>> v31 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::vector<double>> v32 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::mutualInformation(v31, v32);

    std::deque<std::deque<double>> v41 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::deque<double>> v42 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::mutualInformation(v41, v42);

}
BOOST_AUTO_TEST_CASE(mutual_information_int)
{
    /**
     *  This test breaks compilation because of overload of the entropy method for integers is not implemented
     *
     */
    // std::vector<std::vector<int>> v11 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    // std::vector<std::vector<int>> v12 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    // metric::mutualInformation(v11, v12);

    // std::vector<std::deque<int>> v21 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    // std::vector<std::deque<int>> v22 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    // metric::mutualInformation(v21, v22);

    // std::deque<std::vector<int>> v31 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    // std::deque<std::vector<int>> v32 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    // metric::mutualInformation(v31, v32);

    // std::deque<std::deque<int>> v41 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    // std::deque<std::deque<int>> v42 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    // metric::mutualInformation(v41, v42);

}
BOOST_AUTO_TEST_CASE(variation_information) {
    std::vector<std::vector<double>> v11 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::vector<double>> v12 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation(v11, v12);
    metric::variationOfInformation<decltype(v11), metric::Chebyshev<double>>(v11, v12);

    std::vector<std::deque<double>> v21 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::deque<double>> v22 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation(v21, v22);
    metric::variationOfInformation<decltype(v21), metric::Chebyshev<double>>(v21, v22);

    std::deque<std::vector<double>> v31 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::vector<double>> v32 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation(v31, v32);
    metric::variationOfInformation<decltype(v31), metric::Chebyshev<double>>(v31, v32);

    std::deque<std::deque<double>> v41 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::deque<double>> v42 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation(v41, v42);
    metric::variationOfInformation<decltype(v41), metric::Chebyshev<double>>(v41, v42);
}

BOOST_AUTO_TEST_CASE(variation_information_normalized)
{
    std::vector<std::vector<double>> v11 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::vector<double>> v12 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation_normalized(v11, v12);

    std::vector<std::deque<double>> v21 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::deque<double>> v22 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation_normalized(v21, v22);

    std::deque<std::vector<double>> v31 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::vector<double>> v32 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation_normalized(v31, v32);

    std::deque<std::deque<double>> v41 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::deque<double>> v42 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::variationOfInformation_normalized(v41, v42);

}

BOOST_AUTO_TEST_CASE(VOI) {

    std::vector<std::vector<double>> v11 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::vector<double>> v12 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::VOI<long double>()(v11, v12);
    metric::VOI_normalized<long double>()(v11, v12);
    metric::VOI_kl<long double>()(v11, v12);
    metric::VOI_normalized_kl<long double>()(v11, v12);
    
    std::vector<std::deque<double>> v21 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::deque<double>> v22 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::VOI<long double>()(v21, v22);
    metric::VOI_normalized<long double>()(v21, v22);
    metric::VOI_kl<long double>()(v21, v22);
    metric::VOI_normalized_kl<long double>()(v21, v22);

    std::deque<std::vector<double>> v31 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::vector<double>> v32 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::VOI<long double>()(v31, v32);
    metric::VOI_normalized<long double>()(v31, v32);
    metric::VOI_kl<long double>()(v31, v32);
    metric::VOI_normalized_kl<long double>()(v31, v32);

    std::deque<std::deque<double>> v41 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::deque<std::deque<double>> v42 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    metric::VOI<long double>()(v41, v42);
    metric::VOI_normalized<long double>()(v41, v42);
    metric::VOI_kl<long double>()(v41, v42);
    metric::VOI_normalized_kl<long double>()(v41, v42);
}
