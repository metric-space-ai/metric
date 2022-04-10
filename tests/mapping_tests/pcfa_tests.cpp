/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <catch2/catch.hpp>

#include <cmath>
#include <deque>
#include <vector>

#include "metric/mapping/PCFA.hpp"
#include "metric/utils/metric_err.hpp"

TEMPLATE_TEST_CASE("PCFA", "[mapping]", std::deque<float>, std::deque<double>, std::vector<float>, std::vector<double>)
{

        TestType d0_blaze{0, 1, 2};
        TestType d1_blaze{0, 1, 3};
        std::vector<TestType> d_train = {d0_blaze, d1_blaze};

        using RecType = TestType;

        auto pcfa0 = metric::PCFA<RecType, void>(d_train, 2);
        // auto pcfa = metric::PCFA_factory(d_train, 2);  // we also can use factory for autodeduction

        auto weights = pcfa0.weights();
        auto bias = pcfa0.average();
        // model saved to vector and matrix
        auto pcfa = metric::PCFA<RecType, void>(weights, bias);
        // model leaded, same as pcfa0

        RecType d2_blaze{0, 1, 4};
        RecType d3_blaze{0, 2, 2};
        std::vector<RecType> d_test = {d0_blaze, d2_blaze, d3_blaze};

        // compress

        typename TestType::value_type maxerr = 0;
        typename TestType::value_type err = 0;

        auto d_compressed = pcfa.encode(d_test);

        std::vector<RecType> t_compressed = {{-0.5, 0}, {1.5, 0}, {-0.5, 1}};
        for (size_t i = 0; i < d_compressed.size(); i++) {
                for (size_t j = 0; j < d_compressed[i].size(); j++) {
                        err = abs(d_compressed[i][j] - t_compressed[i][j]);
                        if (err > maxerr)
                                maxerr = err;
                }
        }
        REQUIRE(maxerr < std::numeric_limits<typename TestType::value_type>::epsilon() * 3);

        // restore

        maxerr = 0;
        err = 0;

        auto d_restored = pcfa.decode(d_compressed);

        std::vector<RecType> t_restored = {{0, 1, 2}, {0, 1, 4}, {0, 2, 2}};

        for (size_t i = 0; i < d_restored.size(); i++) {
                for (size_t j = 0; j < d_restored[i].size(); j++) {
                        err = abs(d_restored[i][j] - t_restored[i][j]);
                        if (err > maxerr)
                                maxerr = err;
                }
        }
        REQUIRE(maxerr < std::numeric_limits<typename TestType::value_type>::epsilon() * 3);

        // eigenmodes

        maxerr = 0;
        err = 0;

        std::vector<RecType> t_eigenmodes = {{0, 1, 2.5}, {0, 0, 1}, {0, 1, 0}};

        auto d_eigenmodes = pcfa.eigenmodes();

        for (size_t i = 0; i < d_eigenmodes.size(); i++) {
                for (size_t j = 0; j < d_eigenmodes[i].size(); j++) {
                        err = abs(d_eigenmodes[i][j] - t_eigenmodes[i][j]);
                        if (err > maxerr)
                                maxerr = err;
                }
        }
        REQUIRE(maxerr < std::numeric_limits<typename TestType::value_type>::epsilon() * 3);
}
