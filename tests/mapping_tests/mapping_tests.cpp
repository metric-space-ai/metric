/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#include <catch2/catch.hpp>

#include <algorithm>

#include "modules/distance.hpp"

#include "modules/utils/graph.hpp"

using namespace Catch::literals;

TEMPLATE_TEST_CASE("Metric", "[mapping]", float, double)
{
    using Vector = std::vector<TestType>;
    metric::Euclidean<TestType> metric;

    Vector vNull = {};
    Vector v0 = { 0, 0, 0, 0, 0 };
    Vector v7 = { 7, 7, 7.5, 7, 7 };
    Vector v = { 1, 2.3, -2.7, 0, 3 };

    //TestType t = 0.0000001;
    REQUIRE(metric(vNull, vNull) == 0_a);
    REQUIRE(metric(v0, v0) == 0_a);
    REQUIRE(metric(v7, v) == 15.070832757349542_a);
}

TEST_CASE("Grid4", "[mapping]")
{
    metric::Grid4 grid5(5);
    REQUIRE_FALSE(grid5.isValid());

    metric::Grid4 grid25(25);
    REQUIRE(grid25.isValid());
    REQUIRE(grid25.getNodesNumber() == 25);

    metric::Grid4 grid32(3, 2);
    REQUIRE(grid32.isValid());
    REQUIRE(grid32.getNodesNumber() == 6);

    auto neighboursList = grid25.getNeighbours(9, 3);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
    REQUIRE(neighboursList[0] == neighbours0);

    std::vector<size_t> neighbours1 = { 4, 8, 14 };
	REQUIRE(neighboursList[1] == neighbours1);

	std::vector<size_t> neighbours2 = { 3, 7, 13, 19 };
	REQUIRE(neighboursList[2] == neighbours2);
}

TEST_CASE("Grid6", "[mapping]")
{
    metric::Grid6 grid5(5);
    REQUIRE_FALSE(grid5.isValid());

    metric::Grid6 grid25(25);
    REQUIRE(grid25.isValid());
    REQUIRE(grid25.getNodesNumber() == 25);

    metric::Grid6 grid30(6, 5);
    REQUIRE(grid30.isValid());
    REQUIRE(grid30.getNodesNumber() == 30);

    auto neighboursList = grid30.getNeighbours(12, 3);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 12 };
	REQUIRE(neighboursList[0] == neighbours0);


	std::vector<size_t> neighbours1 = { 6, 13, 18 };
	REQUIRE(neighboursList[1] == neighbours1);


	std::vector<size_t> neighbours2 = { 0, 1, 7, 14, 19, 24, 25 };
	REQUIRE(neighboursList[2] == neighbours2);
}
TEST_CASE("Grid8", "[mapping]")
{
    metric::Grid8 grid5(5);
    REQUIRE_FALSE(grid5.isValid());

    metric::Grid8 grid25(25);
    REQUIRE(grid25.isValid());
    REQUIRE(grid25.getNodesNumber() == 25);

    metric::Grid8 grid6(3, 2);
    REQUIRE(grid6.isValid());
    REQUIRE(grid6.getNodesNumber() == 6);

    auto neighboursList = grid25.getNeighbours(9, 3);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
	REQUIRE(neighboursList[0] == neighbours0);

    std::vector<size_t> neighbours1 = { 3, 4, 8, 13, 14 };
	REQUIRE(neighboursList[1] == neighbours1);

    std::vector<size_t> neighbours2 = { 2, 7, 12, 17, 18, 19 };
	REQUIRE(neighboursList[2] == neighbours2);

    std::vector<size_t> neighbours3 = { 1, 6, 11, 16, 21, 22, 23, 24 };
	REQUIRE(neighboursList[3] == neighbours3);
}

TEST_CASE("LPS", "[mapping]")
{
    metric::LPS lps5(5);
    REQUIRE(lps5.isValid());

    metric::LPS lps(11);
    REQUIRE(lps.isValid());

    auto neighboursList = lps.getNeighbours(9, 2);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
	REQUIRE(neighboursList[0] == neighbours0);

    std::vector<size_t> neighbours1 = { 5, 8, 10 };
	REQUIRE(neighboursList[1] == neighbours1);

    std::vector<size_t> neighbours2 = { 0, 4, 6, 7 };
	REQUIRE(neighboursList[2] == neighbours2);

    /* LPS(41) */
    metric::LPS lps41(41);
    REQUIRE(lps41.isValid());

    neighboursList = lps41.getNeighbours(9, 2);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    neighbours0 = { 9 };
	REQUIRE(neighboursList[0] == neighbours0);

    neighbours1 = { 8, 10, 32 };
	REQUIRE(neighboursList[1] == neighbours1);

    neighbours2 = { 7, 11, 31, 33, 36, 37 };
	REQUIRE(neighboursList[2] == neighbours2);
}

TEST_CASE("Paley", "[mapping]")
{
    metric::Paley paley12(12);
    REQUIRE_FALSE(paley12.isValid());

    metric::Paley paley13(13);
    REQUIRE(paley13.isValid());

    auto neighboursList = paley13.getNeighbours(9, 1);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
	REQUIRE(neighboursList[0] == neighbours0);

    std::vector<size_t> neighbours1 = { 0, 5, 6, 8, 10, 12 };
	REQUIRE(neighboursList[1] == neighbours1);
}

TEST_CASE("Margulis", "[mapping]")
{
    metric::Margulis margulis5(5);
    REQUIRE_FALSE(margulis5.isValid());

    metric::Margulis margulis25(25);
    REQUIRE(margulis25.isValid());

    auto neighboursList = margulis25.getNeighbours(7, 1);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 7 };
	REQUIRE(neighboursList[0] == neighbours0);

    std::vector<size_t> neighbours1 = { 2, 5, 9, 12 };
	REQUIRE(neighboursList[1] == neighbours1);
}
