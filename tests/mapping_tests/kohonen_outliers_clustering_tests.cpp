/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <chrono>

#include "modules/mapping.hpp"

#define BOOST_TEST_MODULE Main

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(KOC) {
	
	using Record = std::vector<double>;
				
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
	double sigma = 1.5;
		
	metric::KOC_factory<Record, metric::Grid4> simple_koc_factory(best_w_grid_size, best_h_grid_size, sigma, 0.5, 0.0, 300);    
	auto simple_koc = simple_koc_factory(dataset, num_clusters); 


	// original dataset

	auto anomalies = simple_koc.check_if_anomaly(dataset);	

	// there are should be no anomalies in the train dataset
	for (int i = 0; i < anomalies.size(); i++)
	{
		BOOST_CHECK(anomalies[i] == 0);
	}
	
	auto assignments = simple_koc.assign_to_clusters(dataset);	

	// there are should be no zero-named clusters in the train dataset
	for (int i = 0; i < assignments.size(); i++)
	{
		BOOST_CHECK(assignments[i] != 0);
	}


	// test dataset

	anomalies = simple_koc.check_if_anomaly(test_set);	
	// first two records should be anomalies, others two should not
	BOOST_CHECK(anomalies[0] == 1);
	BOOST_CHECK(anomalies[1] == 1);
	BOOST_CHECK(anomalies[2] == 0);
	BOOST_CHECK(anomalies[3] == 0);
	
	assignments = simple_koc.assign_to_clusters(test_set);	
	// first two records should zero-named anomalies
	BOOST_CHECK(assignments[0] == 0);
	BOOST_CHECK(assignments[1] == 0);

	// top outliers

	auto [idxs, sorted_distances] = simple_koc.top_outliers(test_set, 10);
	// indexes should be as following
	BOOST_CHECK(idxs[0] == 1);
	BOOST_CHECK(idxs[1] == 0);
	BOOST_CHECK(idxs[2] == 2);
	BOOST_CHECK(idxs[3] == 3);

	///////////////////////////////////////////////////////////////
	
	// empty test dataset
	
	std::vector<Record> empty_set;
			
	metric::KOC_factory<Record, metric::Grid4> koc_factory_1(best_w_grid_size, best_h_grid_size, sigma, 0.5, 0.0, 300);    
	auto koc_1 = koc_factory_1(dataset, num_clusters); 

	anomalies = koc_1.check_if_anomaly(empty_set);
	// all should be ok, but zero sized result
	BOOST_CHECK(anomalies.size() == 0);
	
	assignments = koc_1.assign_to_clusters(empty_set);
	// all should be ok, but zero sized result
	BOOST_CHECK(assignments.size() == 0);

	//top outliers

	std::tie(idxs, sorted_distances) = koc_1.top_outliers(empty_set, 10);
	// all should be ok, but zero sized result
	BOOST_CHECK(idxs.size() == 0);
	BOOST_CHECK(sorted_distances.size() == 0);
}

