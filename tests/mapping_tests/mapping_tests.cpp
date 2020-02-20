/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include <algorithm>

#include "modules/distance.hpp"

#include "modules/utils/graph.hpp"

#define BOOST_TEST_MODULE Main
//#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(Metric)
{
    using Vector = std::vector<double>;
    metric::Euclidian<double> metric;

    Vector vNull = {};
    Vector v0 = { 0, 0, 0, 0, 0 };
    Vector v7 = { 7, 7, 7.5, 7, 7 };
    Vector v = { 1, 2.3, -2.7, 0, 3 };

    double t = 0.0000001;
    BOOST_CHECK_CLOSE(metric(vNull, vNull), 0, t);
    BOOST_CHECK_CLOSE(metric(v0, v0), 0, t);
    BOOST_CHECK_CLOSE(metric(v7, v), 15.070832757349542, t);
}

BOOST_AUTO_TEST_CASE(Grid4)
{
    metric::Grid4 grid5(5);  // replaced everywhere mapping::SOM_details with graph by Max F, 2019-05-16
    BOOST_CHECK(!grid5.isValid());

    metric::Grid4 grid25(25);
    BOOST_CHECK(grid25.isValid());
    BOOST_CHECK_EQUAL(grid25.getNodesNumber(), 25);

    metric::Grid4 grid32(3, 2);
    BOOST_CHECK(grid32.isValid());
    BOOST_CHECK_EQUAL(grid32.getNodesNumber(), 6);

    auto neighboursList = grid25.getNeighbours(9, 3);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[0].begin(), neighboursList[0].end(), neighbours0.begin(), neighbours0.end());

    std::vector<size_t> neighbours1 = { 4, 8, 14 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[1].begin(), neighboursList[1].end(), neighbours1.begin(), neighbours1.end());

    std::vector<size_t> neighbours2 = { 3, 7, 13, 19 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[2].begin(), neighboursList[2].end(), neighbours2.begin(), neighbours2.end());
}

BOOST_AUTO_TEST_CASE(Grid6)
{
    metric::Grid6 grid5(5);
    BOOST_CHECK(!grid5.isValid());

    metric::Grid6 grid25(25);
    BOOST_CHECK(grid25.isValid());
    BOOST_CHECK_EQUAL(grid25.getNodesNumber(), 25);

    metric::Grid6 grid30(6, 5);
    BOOST_CHECK(grid30.isValid());
    BOOST_CHECK_EQUAL(grid30.getNodesNumber(), 30);

    auto neighboursList = grid30.getNeighbours(12, 3);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 12 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[0].begin(), neighboursList[0].end(), neighbours0.begin(), neighbours0.end());

    std::vector<size_t> neighbours1 = { 6, 13, 18 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[1].begin(), neighboursList[1].end(), neighbours1.begin(), neighbours1.end());

    std::vector<size_t> neighbours2 = { 0, 1, 7, 14, 19, 24, 25 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[2].begin(), neighboursList[2].end(), neighbours2.begin(), neighbours2.end());
}
BOOST_AUTO_TEST_CASE(Grid8)
{
    metric::Grid8 grid5(5);
    BOOST_CHECK(!grid5.isValid());

    metric::Grid8 grid25(25);
    BOOST_CHECK(grid25.isValid());
    BOOST_CHECK_EQUAL(grid25.getNodesNumber(), 25);

    metric::Grid8 grid6(3, 2);
    BOOST_CHECK(grid6.isValid());
    BOOST_CHECK_EQUAL(grid6.getNodesNumber(), 6);

    auto neighboursList = grid25.getNeighbours(9, 3);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[0].begin(), neighboursList[0].end(), neighbours0.begin(), neighbours0.end());

    std::vector<size_t> neighbours1 = { 3, 4, 8, 13, 14 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[1].begin(), neighboursList[1].end(), neighbours1.begin(), neighbours1.end());

    std::vector<size_t> neighbours2 = { 2, 7, 12, 17, 18, 19 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[2].begin(), neighboursList[2].end(), neighbours2.begin(), neighbours2.end());

    std::vector<size_t> neighbours3 = { 1, 6, 11, 16, 21, 22, 23, 24 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[3].begin(), neighboursList[3].end(), neighbours3.begin(), neighbours3.end());
}

BOOST_AUTO_TEST_CASE(LPS)
{
    metric::LPS lps5(5);
    BOOST_CHECK_EQUAL(lps5.isValid(), true);

    metric::LPS lps(11);
    BOOST_CHECK_EQUAL(lps.isValid(), true);

    auto neighboursList = lps.getNeighbours(9, 2);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[0].begin(), neighboursList[0].end(), neighbours0.begin(), neighbours0.end());

    std::vector<size_t> neighbours1 = { 5, 8, 10 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[1].begin(), neighboursList[1].end(), neighbours1.begin(), neighbours1.end());

    std::vector<size_t> neighbours2 = { 0, 4, 6, 7 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[2].begin(), neighboursList[2].end(), neighbours2.begin(), neighbours2.end());

    /* LPS(41) */
    metric::LPS lps41(41);
    BOOST_CHECK_EQUAL(lps41.isValid(), true);

    neighboursList = lps41.getNeighbours(9, 2);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    neighbours0 = { 9 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[0].begin(), neighboursList[0].end(), neighbours0.begin(), neighbours0.end());

    neighbours1 = { 8, 10, 32 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[1].begin(), neighboursList[1].end(), neighbours1.begin(), neighbours1.end());

    neighbours2 = { 7, 11, 31, 33, 36, 37 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[2].begin(), neighboursList[2].end(), neighbours2.begin(), neighbours2.end());
}

BOOST_AUTO_TEST_CASE(Paley)
{
    metric::Paley paley12(12);
    BOOST_CHECK_EQUAL(paley12.isValid(), false);

    metric::Paley paley13(13);
    BOOST_CHECK_EQUAL(paley13.isValid(), true);

    auto neighboursList = paley13.getNeighbours(9, 1);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 9 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[0].begin(), neighboursList[0].end(), neighbours0.begin(), neighbours0.end());

    std::vector<size_t> neighbours1 = { 0, 5, 6, 8, 10, 12 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[1].begin(), neighboursList[1].end(), neighbours1.begin(), neighbours1.end());
}

BOOST_AUTO_TEST_CASE(Margulis)
{
    metric::Margulis margulis5(5);
    BOOST_CHECK_EQUAL(margulis5.isValid(), false);

    metric::Margulis margulis25(25);
    BOOST_CHECK_EQUAL(margulis25.isValid(), true);

    auto neighboursList = margulis25.getNeighbours(7, 1);
    for (auto& neighbours : neighboursList) {
        std::sort(neighbours.begin(), neighbours.end());
    }

    std::vector<size_t> neighbours0 = { 7 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[0].begin(), neighboursList[0].end(), neighbours0.begin(), neighbours0.end());

    std::vector<size_t> neighbours1 = { 2, 5, 9, 12 };
    BOOST_CHECK_EQUAL_COLLECTIONS(
        neighboursList[1].begin(), neighboursList[1].end(), neighbours1.begin(), neighbours1.end());
}
