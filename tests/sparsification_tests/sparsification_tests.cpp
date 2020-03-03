/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include "modules/utils/graph/sparsify.hpp"

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(kruskal_empty_graph)
{
    blaze::CompressedMatrix<float, blaze::columnMajor> input(0, 0);
    blaze::CompressedMatrix<float, blaze::columnMajor> output =
        metric::kruskal_sparsify(input);

    BOOST_TEST(output.rows() == 0);
    BOOST_TEST(output.columns() == 0);
}


BOOST_AUTO_TEST_CASE(kruskal_not_connected_nodes)
{
    blaze::CompressedMatrix<float, blaze::columnMajor> input(10, 10);
    input.reserve(0);
    blaze::CompressedMatrix<float, blaze::columnMajor> output =
        metric::kruskal_sparsify(input);

    BOOST_TEST(output.rows() == 10);
    BOOST_TEST(output.columns() == 10);
    BOOST_TEST(output.nonZeros() == 0);
}

BOOST_AUTO_TEST_CASE(kruskal_two_nodes_one_connection)
{
    blaze::CompressedMatrix<float, blaze::columnMajor> input(2, 2);
    input.reserve(1);
    input(0, 1) = 5;
    blaze::CompressedMatrix<float, blaze::columnMajor> output =
        metric::kruskal_sparsify(input);

    BOOST_TEST(output.rows() == 2);
    BOOST_TEST(output.columns() == 2);
    BOOST_TEST(output.nonZeros() == 2);
    BOOST_TEST(output(1, 0) == 5);
    BOOST_TEST(output(0, 1) == 5);
}

BOOST_AUTO_TEST_CASE(kruskal_two_trees)
{

    blaze::CompressedMatrix<float, blaze::columnMajor> input(4, 4);
    input.reserve(4);
    input(0, 1) = 1;
    input(1, 2) = 100;
    input(2, 3) = 2;
    input(0, 3) = 200;

    blaze::CompressedMatrix<float, blaze::columnMajor> output =
        metric::kruskal_sparsify(input);

    BOOST_TEST(output.rows() == 4);
    BOOST_TEST(output.columns() == 4);
    BOOST_TEST(output.nonZeros() == 6);
    BOOST_TEST(output(1, 0) == 1);
    BOOST_TEST(output(3, 2) == 2);
    BOOST_TEST(output(2, 1) == 100);
    BOOST_TEST(output(0, 1) == 1);
    BOOST_TEST(output(2, 3) == 2);
    BOOST_TEST(output(1, 2) == 100);
}

BOOST_AUTO_TEST_CASE(kruskal_matrix_creation_order)
{
    blaze::CompressedMatrix<float, blaze::columnMajor> input(4, 4);
    input.reserve(4);

    input.append(0, 1, 1);
    input.finalize(1);
    input.append(1, 2, 100);
    input.finalize(2);
    input.append(2, 3, 2);    
    input.append(0, 3, 200);        
    input.finalize(3);

    blaze::CompressedMatrix<float, blaze::columnMajor> output =
        metric::kruskal_sparsify(input);

    BOOST_TEST(output.rows() == 4);
    BOOST_TEST(output.columns() == 4);
    BOOST_TEST(output.nonZeros() == 6);
    BOOST_TEST(output(1, 0) == 1);
    BOOST_TEST(output(3, 2) == 2);
    BOOST_TEST(output(2, 1) == 100);
    BOOST_TEST(output(0, 1) == 1);
    BOOST_TEST(output(2, 3) == 2);
    BOOST_TEST(output(1, 2) == 100);
}


BOOST_AUTO_TEST_CASE(kruskal_maximum_spanning_tree)
{

    blaze::CompressedMatrix<float, blaze::columnMajor> input(4, 4);
    input.reserve(4);
    input(0, 1) = 1;
    input(1, 2) = 100;
    input(2, 3) = 2;
    input(0, 3) = 200;

    blaze::CompressedMatrix<float, blaze::columnMajor> output =
        metric::kruskal_sparsify(input, false);

    BOOST_TEST(output.rows() == 4);
    BOOST_TEST(output.columns() == 4);
    BOOST_TEST(output.nonZeros() == 6);
    BOOST_TEST(output(3, 2) == 2);
    BOOST_TEST(output(2, 1) == 100);
    BOOST_TEST(output(0, 3) == 200);
    BOOST_TEST(output(2, 3) == 2);
    BOOST_TEST(output(1, 2) == 100);
    BOOST_TEST(output(3, 0) == 200);
}

BOOST_AUTO_TEST_CASE(kruskal_input_bad_size)
{
    blaze::CompressedMatrix<float, blaze::columnMajor> input(3, 4);
    input.reserve(1);
    input(0, 1) = 1;

    try {
        blaze::CompressedMatrix<float, blaze::columnMajor> output =
            metric::kruskal_sparsify(input);
        throw std::runtime_error("matrix property of n==m is not checked");
    } catch (std::invalid_argument& e) {
        // nothing here
    } 
}
