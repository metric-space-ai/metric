/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <iostream>
#include "../../modules/utils/graph.hpp"
#include "../../modules/distance.hpp"


int main()
{
	std::cout << "Graph space example have started" << std::endl;
	std::cout << "" << std::endl;
	
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
    auto g_0 = metric::KNNGraph<std::vector<double>, metric::Euclidian<double>>(table, neighbors_num, 2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_0 = g_0.gnnn_search(query, search_neighbors_num);
    std::sort(found_0.begin(), found_0.end());
	print_vector(found_0);
	assert(found_0.size() == 3);
	// we expect only 4, 5, 6, 13 or 14 nodes (depends on random)
	assert(found_0[0] == 4 || found_0[0] == 5 || found_0[0] == 6 || found_0[0] == 13 || found_0[0] == 14 );
	assert(found_0[1] == 4 || found_0[1] == 5 || found_0[1] == 6 || found_0[1] == 13 || found_0[1] == 14 );
	assert(found_0[2] == 4 || found_0[2] == 5 || found_0[2] == 6 || found_0[2] == 13 || found_0[2] == 14 );
    std::cout << std::endl;


	///	

	// negative params
    auto g_1 = metric::KNNGraph<std::vector<double>, metric::Euclidian<double>>(table, -neighbors_num, -2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_1 = g_1.gnnn_search(query, search_neighbors_num);
	print_vector(found_1);
	assert(found_1.size() > 0);
    std::cout << std::endl;

	// negative params for search, nothing bad should be happened too
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	found_1 = g_1.gnnn_search(query, -search_neighbors_num, -10);
	print_vector(found_1);
	assert(found_1.size() == 0);
    std::cout << std::endl;
	

	///

	// params more than nodes exists
    auto g_2 = metric::KNNGraph<std::vector<double>, metric::Euclidian<double>>(table, table.size() * 10, table.size() * 25);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_2 = g_2.gnnn_search(query, table.size() * 50);
	print_vector(found_2);
	assert(found_2.size() > 0);
    std::cout << std::endl;
	

	///

	// empty dataset
	auto g_3 = metric::KNNGraph<std::vector<double>, metric::Euclidian<double>>({}, neighbors_num, 2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_3 = g_3.gnnn_search(query, search_neighbors_num);
	print_vector(found_3);
	assert(found_3.size() == 0);
    std::cout << std::endl;


	///

	// dataset and query with different dimensions
	auto g_4 = metric::KNNGraph<std::vector<double>, metric::Euclidian<double>>({}, neighbors_num, 2.5 * neighbors_num);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_4 = g_4.gnnn_search({ 7, 5, 7, 5, 7, 5, 7, 5 }, search_neighbors_num);
	print_vector(found_4);
	assert(found_4.size() == 0);
    std::cout << std::endl;


	///
	
	//	
    auto g_5 = metric::KNNGraph(g_0);

	// nothing bad should be happened
    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found_5 = g_5.gnnn_search(query, search_neighbors_num);
	print_vector(found_4);
	assert(found_4.size() == found_0.size());
    std::cout << std::endl;

	
    return 0;
}
