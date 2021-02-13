/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vector>
#include <chrono>

#include "modules/mapping.hpp"


TEMPLATE_TEST_CASE("Main", "[mapping]", float, double) {
	
	using Record = std::vector<TestType>;
				
	size_t best_w_grid_size = 3;
	size_t best_h_grid_size = 2;	

	// init

	std::vector<Record> dataset = {

		{0, 0.1},
		{0.2, 0},

		{0, 1.2},
		{0.1, 1},

		{0.1, 2},
		{0.2, 2},

		{1, 0},
		{1.2, 0.1},

		{1.3, 1.1},
		{0.9, 1},

		{1.1, 2},
		{0.9, 1.9},
	};

	std::vector<Record> test_set = {
		{0.5, 0.5},
		{5, 5},
		{0, 0.1},
		{0, 1},
	};
	
	int num_clusters = 3;
	TestType sigma = 1.5;
	TestType random_seed = 777;
		
	metric::KOC_factory<Record, metric::Grid4> simple_koc_factory(best_w_grid_size, best_h_grid_size, sigma, 0.5, 0.0, 300, -1, 1, 2, 0.5, random_seed);    
	auto simple_koc = simple_koc_factory(dataset, num_clusters); 

	// original dataset

	auto anomalies = simple_koc.check_if_anomaly(dataset);	

	// there are should be no anomalies in the train dataset
	for (int i = 0; i < anomalies.size(); i++)
	{
		REQUIRE(anomalies[i] == 0);
	}
	
	auto assignments = simple_koc.assign_to_clusters(dataset);	

	// there are should be no zero-named clusters in the train dataset
	for (int i = 0; i < assignments.size(); i++)
	{
		REQUIRE(assignments[i] != 0);
	}


	// test dataset

	anomalies = simple_koc.check_if_anomaly(test_set);	
	// first two records should be anomalies, others two should not
	REQUIRE(anomalies[0] == 1);
	REQUIRE(anomalies[1] == 1);
	REQUIRE(anomalies[2] == 0);
	REQUIRE(anomalies[3] == 0);
	
	assignments = simple_koc.assign_to_clusters(test_set);	
	// first two records should zero-named anomalies
	REQUIRE(assignments[0] == 0);
	REQUIRE(assignments[1] == 0);

	// top outliers

	auto [idxs, sorted_distances] = simple_koc.top_outliers(test_set, 10);
	// indexes should be as following
	REQUIRE(idxs[0] == 1);
	REQUIRE(idxs[1] == 0);
	REQUIRE(idxs[2] == 2);
	REQUIRE(idxs[3] == 3);
}

TEMPLATE_TEST_CASE("Empty", "[mapping]", float, double) {
	
	using Record = std::vector<TestType>;
				
	size_t best_w_grid_size = 3;
	size_t best_h_grid_size = 2;	

	// init

	std::vector<Record> dataset = {

		{0, 0.1},
		{0.2, 0},

		{0, 1.2},
		{0.1, 1},

		{0.1, 2},
		{0.2, 2},

		{1, 0},
		{1.2, 0.1},

		{1.3, 1.1},
		{0.9, 1},

		{1.1, 2},
		{0.9, 1.9},
	};

	std::vector<Record> test_set = {
		{0.5, 0.5},
		{5, 5},
		{0, 0.1},
		{0, 1},
	};
	
	int num_clusters = 3;
	TestType sigma = 1.5;
	TestType random_seed = 777;
	
	// empty test dataset
	
	std::vector<Record> empty_set;
			
	metric::KOC_factory<Record, metric::Grid4> koc_factory_1(best_w_grid_size, best_h_grid_size, sigma, 0.5, 0.0, 300, -1, 1, 2, 0.5, random_seed);    
	auto koc_1 = koc_factory_1(dataset, num_clusters); 

	auto anomalies = koc_1.check_if_anomaly(empty_set);
	// all should be ok, but zero sized result
	REQUIRE(anomalies.size() == 0);
	
	auto assignments = koc_1.assign_to_clusters(empty_set);
	// all should be ok, but zero sized result
	REQUIRE(assignments.size() == 0);

	//top outliers

	auto [idxs, sorted_distances] = koc_1.top_outliers(empty_set, 10);
	// all should be ok, but zero sized result
	REQUIRE(idxs.size() == 0);
	REQUIRE(sorted_distances.size() == 0);
}

