/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include "modules/utils/type_traits.hpp"
#include "modules/correlation/entropy.hpp"
#include "modules/distance/k-structured/Edit.hpp"
#include "modules/distance/k-related/Standards.hpp"

#define BOOST_TEST_DYN_LINK // added by Max F
#define BOOST_TEST_MODULE vmixing_test
#include <boost/test/unit_test.hpp>

#include <vector>
#include <deque>
#include <array>
#include <numeric>
#include <stdexcept>
//#include <limits> // only for cout precision

//typedef std::numeric_limits< double > dbl; // only for cout precision

template <typename T>
bool float_eq(T l, T r) {
    return std::abs(l -r) <= std::numeric_limits<T>::epsilon() * 50; // relaxed, but still unstable, TODO improve
}
BOOST_AUTO_TEST_CASE(entropy) {
    std::vector<std::vector<double>>  v1 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::vector<std::deque<double>>   v2 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::vector<std::array<double,2>> v3 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::deque<std::vector<double>>   v4 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    std::deque<std::array<double, 2>> v7 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    using Metric = metric::Chebyshev<double>;
    //std::cout.precision(dbl::max_digits10);
    //std::cout << metric::Entropy<void, Metric>(Metric(), 3, 2)(v1) << "\n";
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>(Metric(), 3, 2)(v1), -4.4489104772539489)); //-4.4489104772539374));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>(Metric(), 3, 2)(v2), -4.4489104772539489));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>(Metric(), 3, 2)(v3), -4.4489104772539489));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>(Metric(), 3, 2)(v4), -4.4489104772539489));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>(Metric(), 3, 2)(v7), -4.4489104772539489));
    //std::cout.precision(dbl::max_digits10);
    //std::cout << metric::Entropy<void, Metric>()(v1) << "\n";
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>()(v1), -5.3989104772539491)); //-5.3989104772539376)); // -5.3989104772539225));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>()(v2), -5.3989104772539491));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>()(v3), -5.3989104772539491));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>()(v4), -5.3989104772539491));
    BOOST_TEST(float_eq(metric::Entropy<void, Metric>()(v7), -5.3989104772539491));

    std::vector<std::string> v8 = { "AAA", "HJGJHFG", "BBB", "AAAA", "long long long long long long string", "abcdefghjklmnopqrstuvwxyz" };
    //std::cout.precision(dbl::max_digits10);
    //std::cout << metric::Entropy<void, metric::Edit<int>>(metric::Edit<int>(), 3, 2.0)(v8) << "\n";
    BOOST_TEST(float_eq(metric::Entropy<void, metric::Edit<int>>(metric::Edit<int>(), 3, 2.0)(v8), -9.3586210470159283)); //0.58333333333333337));
}

BOOST_AUTO_TEST_CASE(vmixing) {

    std::vector<std::vector<double>> v11 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 0 } };
    std::vector<std::vector<double>> v12 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 0 } };
    auto vms = metric::VMixing_simple<void, metric::Euclidean<double>>();
    auto vm = metric::VMixing<void, metric::Euclidean<double>>();
    //std::cout.precision(dbl::max_digits10);
    //std::cout <<
                 ////metric::VOI_simple(v11, v12) << "\n" <<
                 //vms(v11, v12) << "\n" <<
                 ////metric::VOI(v11, v12) << "\n" <<
                 //vm(v11, v12);
    //BOOST_TEST(float_eq(metric::VOI_simple(v11, v12), 11.813781191217037));
    BOOST_TEST(float_eq(vms(v11, v12), 0.0608977897890286)); //0.75859128220107586));
    //BOOST_TEST(float_eq(metric::VOI(v11, v12), -34.913083805038639)); //-31.36271401282686));
    BOOST_TEST(float_eq(vm(v11, v12), 11.821247820859364)); //12.70867240400823)); //12.708672404008247));

    std::vector<std::deque<double>> v21 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 0 } };
    std::vector<std::deque<double>> v22 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 0 } };
    //BOOST_TEST(float_eq(metric::VOI_simple(v21, v22), 11.813781191217037));
    BOOST_TEST(float_eq(vms(v21, v22), 0.0608977897890286));
    //BOOST_TEST(float_eq(metric::VOI(v21, v22), -34.913083805038639));
    BOOST_TEST(float_eq(vm(v21, v22), 11.821247820859364));

    std::deque<std::vector<double>> v31 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 0 } };
    std::deque<std::vector<double>> v32 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 0 } };
    //BOOST_TEST(float_eq(metric::VOI_simple(v31, v32), 11.813781191217037));
    BOOST_TEST(float_eq(vms(v31, v32), 0.0608977897890286));
    //BOOST_TEST(float_eq(metric::VOI(v31, v32), -34.913083805038639));
    BOOST_TEST(float_eq(vm(v31, v32), 11.821247820859364));

    std::deque<std::deque<double>> v41 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 0 } };
    std::deque<std::deque<double>> v42 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 0 } };
    //BOOST_TEST(float_eq(metric::VOI_simple(v41, v42), 11.813781191217037));
    BOOST_TEST(float_eq(vms(v41, v42), 0.0608977897890286));
    //BOOST_TEST(float_eq(metric::VOI(v41, v42), -34.913083805038639));
    BOOST_TEST(float_eq(vm(v41, v42), 11.821247820859364));

    std::vector<std::vector<double>> ds1 {{0, 1, 0}, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0},
                                          {0, 1, 0}, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0},
                                          {0, 1, 0}, {0, 0, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}};
    std::vector<std::vector<double>> ds2 {{0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {2, 1, 0}, {0, 0, 0},
                                          {0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {2, 1, 0}, {0, 0, 0},
                                          {0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {2, 1, 0}, {0, 0, 0}};
    //std::cout.precision(dbl::max_digits10);
    //std::cout << vms.estimate(ds1, ds2, 5) << "\n";
    //std::cout << vm.estimate(ds1, ds2, 5) << "\n";
    BOOST_TEST(float_eq(vms.estimate(ds1, ds2, 5), 1.2722224834467826));
    BOOST_TEST(float_eq(vm.estimate(ds1, ds2, 5), 0.096469697241235622));

}

