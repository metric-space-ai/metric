/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "modules/utils/graph/sparsify.hpp"


TEMPLATE_TEST_CASE("kruskal_empty_graph", "[sparsification]", float, double)
{
    blaze::CompressedMatrix<TestType, blaze::columnMajor> input(0, 0);
    blaze::CompressedMatrix<TestType, blaze::columnMajor> output =
        metric::sparsify_spanning_tree(input);

    REQUIRE(output.rows() == 0);
    REQUIRE(output.columns() == 0);
}


TEMPLATE_TEST_CASE("kruskal_not_connected_nodes", "[sparsification]", float, double)
{
    blaze::CompressedMatrix<TestType, blaze::columnMajor> input(10, 10);
    input.reserve(0);
    blaze::CompressedMatrix<TestType, blaze::columnMajor> output =
        metric::sparsify_spanning_tree(input);

    REQUIRE(output.rows() == 10);
    REQUIRE(output.columns() == 10);
    REQUIRE(output.nonZeros() == 0);
}

TEMPLATE_TEST_CASE("kruskal_two_nodes_one_connection", "[sparsification]", float, double)
{
    blaze::CompressedMatrix<TestType, blaze::columnMajor> input(2, 2);
    input.reserve(1);
    input(0, 1) = 5;
    blaze::CompressedMatrix<TestType, blaze::columnMajor> output =
        metric::sparsify_spanning_tree(input);

    REQUIRE(output.rows() == 2);
    REQUIRE(output.columns() == 2);
    REQUIRE(output.nonZeros() == 2);
    REQUIRE(output(1, 0) == 5);
    REQUIRE(output(0, 1) == 5);
}

TEMPLATE_TEST_CASE("kruskal_two_trees", "[sparsification]", float, double)
{

    blaze::CompressedMatrix<TestType, blaze::columnMajor> input(4, 4);
    input.reserve(4);
    input(0, 1) = 1;
    input(1, 2) = 100;
    input(2, 3) = 2;
    input(0, 3) = 200;

    blaze::CompressedMatrix<TestType, blaze::columnMajor> output =
        metric::sparsify_spanning_tree(input);

    REQUIRE(output.rows() == 4);
    REQUIRE(output.columns() == 4);
    REQUIRE(output.nonZeros() == 6);
    REQUIRE(output(1, 0) == 1);
    REQUIRE(output(3, 2) == 2);
    REQUIRE(output(2, 1) == 100);
    REQUIRE(output(0, 1) == 1);
    REQUIRE(output(2, 3) == 2);
    REQUIRE(output(1, 2) == 100);
}

TEMPLATE_TEST_CASE("kruskal_matrix_creation_order", "[sparsification]", float, double)
{
    blaze::CompressedMatrix<TestType, blaze::columnMajor> input(4, 4);
    input.reserve(4);

    input.append(0, 1, 1);
    input.finalize(1);
    input.append(1, 2, 100);
    input.finalize(2);
    input.append(2, 3, 2);    
    input.append(0, 3, 200);        
    input.finalize(3);

    blaze::CompressedMatrix<TestType, blaze::columnMajor> output =
        metric::sparsify_spanning_tree(input);

    REQUIRE(output.rows() == 4);
    REQUIRE(output.columns() == 4);
    REQUIRE(output.nonZeros() == 6);
    REQUIRE(output(1, 0) == 1);
    REQUIRE(output(3, 2) == 2);
    REQUIRE(output(2, 1) == 100);
    REQUIRE(output(0, 1) == 1);
    REQUIRE(output(2, 3) == 2);
    REQUIRE(output(1, 2) == 100);
}


TEMPLATE_TEST_CASE("kruskal_maximum_spanning_tree", "[sparsification]", float, double)
{

    blaze::CompressedMatrix<TestType, blaze::columnMajor> input(4, 4);
    input.reserve(4);
    input(0, 1) = 1;
    input(1, 2) = 100;
    input(2, 3) = 2;
    input(0, 3) = 200;

    blaze::CompressedMatrix<TestType, blaze::columnMajor> output =
        metric::sparsify_spanning_tree(input, false);

    REQUIRE(output.rows() == 4);
    REQUIRE(output.columns() == 4);
    REQUIRE(output.nonZeros() == 6);
    REQUIRE(output(3, 2) == 2);
    REQUIRE(output(2, 1) == 100);
    REQUIRE(output(0, 3) == 200);
    REQUIRE(output(2, 3) == 2);
    REQUIRE(output(1, 2) == 100);
    REQUIRE(output(3, 0) == 200);
}

TEMPLATE_TEST_CASE("kruskal_input_bad_size", "[sparsification]", float, double)
{
    blaze::CompressedMatrix<TestType, blaze::columnMajor> input(3, 4);
    input.reserve(1);
    input(0, 1) = 1;

    try {
        blaze::CompressedMatrix<TestType, blaze::columnMajor> output =
            metric::sparsify_spanning_tree(input);
        throw std::runtime_error("matrix property of n==m is not checked");
    } catch (std::invalid_argument& e) {
        // nothing here
    } 
}
