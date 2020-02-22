/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include "modules/utils/graph/partition.hpp"

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(bad_chunk_size)
{

    blaze::DynamicMatrix<double> distance_matrix(10, 10, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 5, 50, 0);
   
    BOOST_TEST(error != 0);
}

BOOST_AUTO_TEST_CASE(working_partition_small_size)
{
    blaze::DynamicMatrix<double> distance_matrix(150, 150, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    for (int i = 0; i < distance_matrix.rows(); i++) {
        for (int j = i+1; j < distance_matrix.columns(); j++) {
            distance_matrix(i, j) = abs(i - j);
            distance_matrix(j, i) = distance_matrix(i, j);
        }
    }

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 5, 100, 0);

    BOOST_TEST(error == 0);
    BOOST_TEST(partition_matrix.rows() == 150);

    for (int i = 0; i < partition_matrix.rows(); i++) {
        int row_sum = 0;
        for (int j = 0; j < partition_matrix.columns(); j++) {
            row_sum += partition_matrix(i, j);
        }
        BOOST_TEST(row_sum == 1);
    }
}

BOOST_AUTO_TEST_CASE(working_partition_three_clusters)
{
    blaze::DynamicMatrix<double> distance_matrix(200,200, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    for (int i = 0; i < distance_matrix.rows(); i++) {
        for (int j = i+1; j < distance_matrix.columns(); j++) {
            distance_matrix(i, j) = (i * j) % 3;
            distance_matrix(j, i) = distance_matrix(i, j);
        }
    }

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 1, 100, 0);

    BOOST_TEST(error == 0);
    BOOST_TEST(partition_matrix.rows() == 200);

    for (int i = 0; i < partition_matrix.rows(); i++) {
        int row_sum = 0;
        for (int j = 0; j < partition_matrix.columns(); j++) {
            row_sum += partition_matrix(i, j);
        }
        BOOST_TEST(row_sum == 1);
    }
}

BOOST_AUTO_TEST_CASE(working_partition_no_clusters)
{
    blaze::DynamicMatrix<double> distance_matrix(200, 200, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 1, 100, 0);

    BOOST_TEST(error == 0);
    BOOST_TEST(partition_matrix.rows() == 200);

    for (int i = 0; i < partition_matrix.rows(); i++) {
        BOOST_TEST(partition_matrix(i, 0) == 1);  
    }
}

BOOST_AUTO_TEST_CASE(working_partition_mod_20)
{
    blaze::DynamicMatrix<double> distance_matrix(200, 200, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    for (int i = 0; i < distance_matrix.rows(); i++) {
        for (int j = i+1; j < distance_matrix.columns(); j++) {
            distance_matrix(i, j) = (i * j) % 20;
            distance_matrix(j, i) = distance_matrix(i, j);
        }
    }

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 5, 10, 0);

    BOOST_TEST(error == 0);
    BOOST_TEST(partition_matrix.rows() == 200);

    for (int i = 0; i < partition_matrix.rows(); i++) {
        int row_sum = 0;
        for (int j = 0; j < partition_matrix.columns(); j++) {
            row_sum += partition_matrix(i, j);
        }
        BOOST_TEST(row_sum == 1);
    }
}

BOOST_AUTO_TEST_CASE(corrupted_distance_matrix)
{
    blaze::DynamicMatrix<double> distance_matrix(200, 200, 0);
    blaze::DynamicMatrix<int> partition_matrix;
   
    for (int i = 0; i < distance_matrix.rows(); i++) {
        for (int j = i + 1; j < distance_matrix.columns(); j++) {
            distance_matrix(i, j) = (i * j);
        }
    }

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 5, 10, 0);

    BOOST_TEST(error != 0);
}

BOOST_AUTO_TEST_CASE(large_distances)
{
    blaze::DynamicMatrix<double> distance_matrix(200, 200, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    for (int i = 0; i < distance_matrix.rows(); i++) {
        for (int j = i + 1; j < distance_matrix.columns(); j++) {
            distance_matrix(i, j) = (i * j) % 4 * 1000;
            distance_matrix(j, i) = distance_matrix(i, j);
        }
    }

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 5, 100, 0);
    BOOST_TEST(error == 0);
}

BOOST_AUTO_TEST_CASE(sparse_matrix_0)
{
    blaze::DynamicMatrix<double> distance_matrix(1000, 1000, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    for (int i = 0; i < distance_matrix.rows(); i++) {
        for (int j = i + 1; j < distance_matrix.columns(); j++) {
            if(i * j == 1000) 
            {
                distance_matrix(i, j) = i;
                distance_matrix(j, i) = distance_matrix(i, j);
            }
        }
    }

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 1, 200, 0);

    BOOST_TEST(error == 0);
    BOOST_TEST(partition_matrix.rows() == 1000);
    BOOST_TEST(partition_matrix.columns() == 4);
}

BOOST_AUTO_TEST_CASE(sparse_matrix_1)
{
    blaze::DynamicMatrix<double> distance_matrix(5000, 5000, 0);
    blaze::DynamicMatrix<int> partition_matrix;

    for (int i = 0; i < distance_matrix.rows(); i++) {
        for (int j = i + 1; j < distance_matrix.columns(); j++) {
            if (i * j == 5000)
            {
                distance_matrix(i, j) = 1;
                distance_matrix(j, i) = distance_matrix(i, j);
            }
        }
    }

    int error = metric::perform_graph_partition(distance_matrix, partition_matrix, 1, 600, 0);

    BOOST_TEST(error == 0);
    BOOST_TEST(partition_matrix.rows() == 5000);
    BOOST_TEST(partition_matrix.columns() == 3);
}