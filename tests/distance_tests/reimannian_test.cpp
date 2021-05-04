/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <catch2/catch.hpp>

#include "modules/distance/d-spaced/Riemannian.hpp"

#include <numeric>
#include <stdexcept>

using namespace Catch::literals;


TEMPLATE_TEST_CASE("riemannian_distance", "[distance]", double) {

    blaze::DynamicMatrix<TestType> A {{1, 2, -3}, {0, -2, 4}, {0, 0, 1}};
    blaze::DynamicMatrix<TestType> B {{2, 0, -2}, {0, 1, 1}, {0, 0, 1}};

    auto rd = metric::RiemannianDistance<void, metric::Euclidean<TestType>>();
    REQUIRE(rd.matDistance(A, B) == 0.6931471805599455_a);

    std::vector<std::vector<TestType>> ds1 {{0, 1}, {0, 0}, {1, 1}, {1, 0}};
    std::vector<std::vector<TestType>> ds2 {{0, 0}, {1, 1}, {2, 2}, {2, 1}};
    REQUIRE(rd(ds1, ds2) == 0.8086438137089399_a);
    REQUIRE(rd.estimate(ds1, ds2, 2) == 0.0_a); // TODO run with larger dataset
}


