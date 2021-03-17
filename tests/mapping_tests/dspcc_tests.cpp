/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vector>
#include <deque>
#include <cmath>
//#include <chrono>

#include "../../modules/mapping/DSPCC.hpp"
#include "../../modules/utils/metric_err.hpp"
#include "../../modules/distance/k-related/Standards.hpp" // we use Euclidean metric for mean squared error evaluation



TEMPLATE_TEST_CASE("DSPCC", "[mapping]", std::deque<float>, std::deque<double>, std::vector<float>, std::vector<double>) {

    TestType d0 {0, 1, 2, 3, 4, 5, 6, 100, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    TestType d1 {0, 1, 2, 3, 4, 5, 6, 7,   8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 100};
    std::vector<TestType> d = {d0, d1};

    float freq_time_balance = 0.5; // try values from 0 to 1 (e g 0, 0.5, 1) to get the following portions of freq-domain: 0, 4/9, 8/9
    auto bundle = metric::DSPCC<TestType, void>(d, 4, 2, freq_time_balance, 4);

    auto pre_encoded = bundle.test_public_wrapper_encode(d);
    auto pre_decoded = bundle.test_public_wrapper_decode(pre_encoded);

    auto encoded = bundle.encode(d);
    auto decoded = bundle.decode(encoded);

    typename TestType::value_type maxerr = 0;
    typename TestType::value_type err = 0;
    for (size_t i = 0; i < d.size(); ++i) {
        for (size_t j = 0; j < d[i].size(); ++j) {
            err = abs(d[i][j] - decoded[i][j]);
            if (err > maxerr)
                maxerr = err;
        }
    }

    REQUIRE(maxerr < std::numeric_limits<typename TestType::value_type>::epsilon() * 1e7);

}

