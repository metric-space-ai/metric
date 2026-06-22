/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "assets/test_data.cpp"
#include "metric/metric/catalog.hpp"
#include <chrono>
#include <iostream>
#include <vector>

int main()
{
	/******************** examples for Kolmogorov-Smirnov Distance **************************/
	// WARNING: mtrc::KolmogorovSmirnov is QUARANTINED — it is NOT an admitted true
	// metric (metric_traits<>::law == metric_law::distance). As shipped it violates
	// the triangle inequality, can return values > 1, and silently returns 0 for
	// tied or fully disjoint all-equal samples. Note the sample data below contains
	// repeated values, which makes the underlying PMQ CDF degenerate. This example
	// is for the compatibility surface only; do not treat the result as a metric.
	// See docs/metrics/quarantine-inventory.md.
	// example for picture
	std::cout << "Kolmogorov-Smirnov distance example have started" << std::endl;
	std::cout << "" << std::endl;

	/*** here are some data records ***/
	std::vector<double> samples_1 = {0, 1, 2, 3, 3, 2, 1, 0, 2, 2};
	std::vector<double> samples_2 = {0, 0, 2, 3, 3, 2, 1, 0, 2, 2};
	// std::vector<double> samples_1 = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	// std::vector<double> samples_2 = { 1, 2, 3, 4, 5 };
	//
	mtrc::KolmogorovSmirnov<std::vector<double>, double> distance;

	auto t1 = std::chrono::steady_clock::now();
	auto result1 = distance(samples_1, samples_2);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result1
			  << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
			  << " s)" << std::endl;
	std::cout << "" << std::endl;

	return 0;
}
