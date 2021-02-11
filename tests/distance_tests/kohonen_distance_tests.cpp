/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vector>
#include <iostream>

#include <chrono>
#include "../../modules/distance.hpp"


TEMPLATE_TEST_CASE("kohonen", "[distance]", float, double)
{
	size_t grid_w = 3;
	size_t grid_h = 2;
	
	using Record = std::vector<TestType>;
		
	std::vector<Record> train_dataset = {
		{0, 0},
		{1, 0},
		{2, 0},
		{0, 1},
		{1, 1},
		{2, 1},
	};


	metric::Kohonen<TestType, Record> distance_1(train_dataset, grid_w, grid_h);

	TestType result;
	result = distance_1(train_dataset[0], train_dataset[1]);
	REQUIRE(result > 0); REQUIRE(result < 2);
	result = distance_1(train_dataset[0], train_dataset[2]);
	REQUIRE(result > 1); REQUIRE(result < 3);
	result = distance_1(train_dataset[0], train_dataset[3]);
	REQUIRE(result > 0); REQUIRE(result < 2);
	result = distance_1(train_dataset[0], train_dataset[4]);
	REQUIRE(result > 1); REQUIRE(result < 3);
	result = distance_1(train_dataset[0], train_dataset[5]);
	REQUIRE(result > 2); REQUIRE(result < 4);
	

    using Vector = std::vector<TestType>;
    using Metric = metric::Euclidean<TestType>;
    using Graph = metric::Grid6; 
    using Distribution = std::uniform_real_distribution<TestType>;

	Distribution distr(-1, 1);

    metric::SOM<Vector, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), 0.8, 0.2, 50, distr);

	som_model.train(train_dataset);
	
	metric::Kohonen<TestType, Vector, Graph, Metric> distance_2(som_model, train_dataset);

	result = distance_2(train_dataset[0], train_dataset[1]);
	REQUIRE(result > 0); REQUIRE(result < 2);
	result = distance_2(train_dataset[0], train_dataset[2]);
	REQUIRE(result > 1); REQUIRE(result < 3);
	result = distance_2(train_dataset[0], train_dataset[3]);
	REQUIRE(result > 0); REQUIRE(result < 2);
	result = distance_2(train_dataset[0], train_dataset[4]);
	REQUIRE(result > 1); REQUIRE(result < 3);
	result = distance_2(train_dataset[0], train_dataset[5]);
	REQUIRE(result > 2); REQUIRE(result < 4);
	
    metric::SOM<Vector, Graph, Metric> negative_som_model(Graph(grid_w, grid_h), Metric(), -0.8, -0.2, 50, distr);
	negative_som_model.train(train_dataset);
	
	metric::Kohonen<TestType, Vector, Graph, Metric> distance_3(negative_som_model, train_dataset);

	result = distance_3(train_dataset[0], train_dataset[1]);
	REQUIRE(result > 0); REQUIRE(result < 2);
	result = distance_3(train_dataset[0], train_dataset[2]);
	REQUIRE(result > 1); REQUIRE(result < 3);
	result = distance_3(train_dataset[0], train_dataset[3]);
	REQUIRE(result > 0); REQUIRE(result < 2);
	result = distance_3(train_dataset[0], train_dataset[4]);
	REQUIRE(result > 1); REQUIRE(result < 3);
	result = distance_3(train_dataset[0], train_dataset[5]);
	REQUIRE(result > 2); REQUIRE(result < 4);
}
