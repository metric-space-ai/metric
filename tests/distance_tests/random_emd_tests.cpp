/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team
*/

#include <catch2/catch.hpp>

#include <vector>
#include "modules/distance.hpp"

using namespace Catch::literals;


TEMPLATE_TEST_CASE("basic_use", "[distance]", float, double)
{
	std::vector<TestType> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<TestType> samples_2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	   
    metric::RandomEMD<std::vector<TestType>, TestType> distance;

    auto result = distance(samples_1, samples_2);
	
    //TestType t = 1.0; // 1.0%
    REQUIRE(result == Approx(1.0).margin(0.01));
}

TEMPLATE_TEST_CASE("different_dimensions", "[distance]", float, double)
{
	std::vector<TestType> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<TestType> samples_2 = { 1, 2, 3, 4, 5 };
	   
    metric::RandomEMD<std::vector<TestType>, TestType> distance;

    auto result = distance(samples_1, samples_2);
	
    //TestType t = 1.0; // 1.0%
    REQUIRE(result == Approx(2.18).margin(0.0218));
}

TEMPLATE_TEST_CASE("equal_samples", "[distance]", float, double)
{
	std::vector<TestType> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<TestType> samples_2 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	   
    metric::RandomEMD<std::vector<TestType>, TestType> distance;

    auto result = distance(samples_1, samples_2);
	
    //TestType t = 0.01; // 0.01%
    REQUIRE(result == Approx(0.0));
}

TEMPLATE_TEST_CASE("single_value_distribution", "[distance]", float, double)
{
	std::vector<TestType> samples_1 = { 0, 0, 0, 0, 0, 0, 0 };
	std::vector<TestType> samples_2 = { 1, 1, 1, 1, 1, 1, 1 };
	   
    metric::RandomEMD<std::vector<TestType>, TestType> distance;

    auto result = distance(samples_1, samples_2);
	
    REQUIRE(std::isnan(result));
}

TEMPLATE_TEST_CASE("non_intersect_distribution", "[distance]", float, double)
{
	std::vector<TestType> samples_1 = { 0, 1, 1, 2, 2, 2, 2, 1, 1, 0 };
	std::vector<TestType> samples_2 = { 10, 11, 11, 12, 12, 12, 12, 11, 11, 10 };
	   
    metric::RandomEMD<std::vector<TestType>, TestType> distance;

    auto result = distance(samples_1, samples_2);
	
    REQUIRE(std::isnan(result));
}
