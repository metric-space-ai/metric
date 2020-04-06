/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#define BOOST_TEST_MODULE knn_graph_test
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "modules/space/knn_graph.hpp"
#include "modules/distance.hpp"
namespace std{ 
std::ostream & operator <<(std::ostream & ostr, const std::pair<std::size_t, bool> & p) {
    return ostr << "{" << p.first << ", " << p.second << "}";
}
}
BOOST_AUTO_TEST_CASE(valid_params)
{	
    size_t neighbors_num = 3;
    size_t search_neighbors_num = 3;
		
    std::vector<std::vector<double>> table = {
        { 0, 1 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 3 },
        { 5, 3 },
        { 4, 6 },
        { 5, 1 },
        { 4, 1 },
        { 3, 2 },
        { 0, 3 },
        { 1, 3 },
        { 2, 3 },
        { 6, 6 },
        { 7, 6 },
        { 0, 2 },
        { 0, 9 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
    };
	
	std::vector<double> query = { 7, 7 };
	
	///	

	// valid params
    auto g_0 = metric::KNNGraph<std::vector<double>, metric::Euclidean<double>>(table, neighbors_num, 2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << " --- valid params --- " << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_0 = g_0.gnnn_search(query, search_neighbors_num);
    std::sort(found_0.begin(), found_0.end());
	print_vector(found_0);
	BOOST_CHECK(found_0.size() == 3);
	// we expect only 4, 5, 6, 13 or 14 nodes (depends on random)
	// BOOST_TEST((found_0[0] == 4 || found_0[0] == 5 || found_0[0] == 6 || found_0[0] == 13 || found_0[0] == 14 ));
	// BOOST_TEST((found_0[1] == 4 || found_0[1] == 5 || found_0[1] == 6 || found_0[1] == 13 || found_0[1] == 14 ));
  // BOOST_TEST((found_0[2] == 4 || found_0[2] == 5 || found_0[2] == 6 || found_0[2] == 13 || found_0[2] == 14 ));
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(negative_params)
{
    size_t neighbors_num = 3;
    size_t search_neighbors_num = 3;
		
    std::vector<std::vector<double>> table = {
        { 0, 1 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 3 },
        { 5, 3 },
        { 4, 6 },
        { 5, 1 },
        { 4, 1 },
        { 3, 2 },
        { 0, 3 },
        { 1, 3 },
        { 2, 3 },
        { 6, 6 },
        { 7, 6 },
        { 0, 2 },
        { 0, 9 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
    };
	
	std::vector<double> query = { 7, 7 };

	///	

	// negative params
    auto g_1 = metric::KNNGraph<std::vector<double>, metric::Euclidean<double>>(table, -neighbors_num, -2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << " --- negative params --- " << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_1 = g_1.gnnn_search(query, search_neighbors_num);
	print_vector(found_1);
	BOOST_CHECK(found_1.size() > 0);
    std::cout << std::endl;

	// negative params for search, nothing bad should be happened too
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	found_1 = g_1.gnnn_search(query, -search_neighbors_num, -10);
	print_vector(found_1);
	BOOST_CHECK(found_1.size() == 0);
    std::cout << std::endl;
	
}

BOOST_AUTO_TEST_CASE(params_more_than_nodes_exists)
{
    size_t neighbors_num = 3;
    size_t search_neighbors_num = 3;
		
    std::vector<std::vector<double>> table = {
        { 0, 1 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 3 },
        { 5, 3 },
        { 4, 6 },
        { 5, 1 },
        { 4, 1 },
        { 3, 2 },
        { 0, 3 },
        { 1, 3 },
        { 2, 3 },
        { 6, 6 },
        { 7, 6 },
        { 0, 2 },
        { 0, 9 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
    };
	
	std::vector<double> query = { 7, 7 };

	///

	// params more than nodes exists
    auto g_2 = metric::KNNGraph<std::vector<double>, metric::Euclidean<double>>(table, table.size() * 10, table.size() * 25);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << " --- params more than nodes exists --- " << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_2 = g_2.gnnn_search(query, table.size() * 50);
	print_vector(found_2);
	BOOST_CHECK(found_2.size() > 0);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(empty_dataset)
{
    size_t neighbors_num = 3;
    size_t search_neighbors_num = 3;
		
    std::vector<std::vector<double>> table = {
        { 0, 1 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 3 },
        { 5, 3 },
        { 4, 6 },
        { 5, 1 },
        { 4, 1 },
        { 3, 2 },
        { 0, 3 },
        { 1, 3 },
        { 2, 3 },
        { 6, 6 },
        { 7, 6 },
        { 0, 2 },
        { 0, 9 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
    };
	
	std::vector<double> query = { 7, 7 };

	///

	// empty dataset
	auto g_3 = metric::KNNGraph<std::vector<double>, metric::Euclidean<double>>(std::vector<std::vector<double>>{}, neighbors_num, 2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << " --- empty dataset --- " << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_3 = g_3.gnnn_search(query, search_neighbors_num);
	print_vector(found_3);
	BOOST_CHECK(found_3.size() == 0);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(dataset_and_query_with_different_dimensions)
{
    size_t neighbors_num = 3;
    size_t search_neighbors_num = 3;
		
    std::vector<std::vector<double>> table = {
        { 0, 1 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 3 },
        { 5, 3 },
        { 4, 6 },
        { 5, 1 },
        { 4, 1 },
        { 3, 2 },
        { 0, 3 },
        { 1, 3 },
        { 2, 3 },
        { 6, 6 },
        { 7, 6 },
        { 0, 2 },
        { 0, 9 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
    };
	
	std::vector<double> query = { 7, 7 };

	///

	// dataset and query with different dimensions
	auto g_4 = metric::KNNGraph<std::vector<double>, metric::Euclidean<double>>(std::vector<std::vector<double>>{}, neighbors_num, 2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << " --- dataset and query with different dimensions --- " << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_4 = g_4.gnnn_search({ 7, 5, 7, 5, 7, 5, 7, 5 }, search_neighbors_num);
	print_vector(found_4);
	BOOST_CHECK(found_4.size() == 0);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(copy_constructor)
{
    size_t neighbors_num = 3;
    size_t search_neighbors_num = 3;
		
    std::vector<std::vector<double>> table = {
        { 0, 1 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 3 },
        { 5, 3 },
        { 4, 6 },
        { 5, 1 },
        { 4, 1 },
        { 3, 2 },
        { 0, 3 },
        { 1, 3 },
        { 2, 3 },
        { 6, 6 },
        { 7, 6 },
        { 0, 2 },
        { 0, 9 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
    };
	
	std::vector<double> query = { 7, 7 };

	///
	
	//	
    auto g_0 = metric::KNNGraph<std::vector<double>, metric::Euclidean<double>>(table, neighbors_num, 2.5 * neighbors_num);
	auto found_0 = g_0.gnnn_search(query, search_neighbors_num);

    auto g_5 = metric::KNNGraph(g_0);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << " --- nothing bad should be happened --- " << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_5 = g_5.gnnn_search(query, search_neighbors_num);
	print_vector(found_5);
	assert(found_5.size() == found_0.size());
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(knngraph_insert) {
    std::vector<double> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    auto graph = metric::KNNGraph<double, metric::Euclidean<double>>(data, 3, 8);
    auto nn = graph.gnnn_search(1.1, 1);
    BOOST_TEST(nn.size() == 1);
    auto nn1 = graph.gnnn_search(1.8, 2);
    BOOST_TEST(nn1.size() == 2);

    auto id1 = graph.insert(2.1);
    BOOST_TEST(id1 == 10);
    BOOST_TEST(graph.size() == id1+1);
    BOOST_TEST(graph[id1] == 2.1);
    
    auto nn2 = graph.gnnn_search(1.8, 1);
    BOOST_TEST(nn2.size() == 1);

    std::vector<double> data2 = {4.3, 5.1, 6.2};
    auto id2 = graph.insert(data2);
    BOOST_TEST(id2 == (std::vector<std::size_t>{11,12,13}), boost::test_tools::per_element());

    auto id3 = graph.insert_if(11, 100);
    BOOST_TEST(id3.first == 0);
    BOOST_TEST(id3.second == false);

    std::vector<double> data3 = { 11.5, 14, 10.02};
    auto id4 = graph.insert_if(data3, 1000.5);
    BOOST_TEST(id4 == (std::vector<std::pair<std::size_t, bool>>{{0, false}, {0, false}, {0, false}}), boost::test_tools::per_element());

}
