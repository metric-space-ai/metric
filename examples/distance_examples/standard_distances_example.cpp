/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <chrono>
#include <iostream>
#include <vector>

// #include "metric/numeric/math/CompressedVector.h"  // for Numeric Euclidean overload
// #include "metric/numeric/math/CompressedMatrix.h"  // for Numeric Euclidean overload

#include "metric/metric/catalog.hpp"

int main()
{
	std::cout << "Standard Distances example have started" << std::endl;
	std::cout << "" << std::endl;

	/*** here are some data records ***/
	std::vector<double> v0 = {0, 1, 1, 1, 1, 1, 2, 3};
	std::vector<double> v1 = {1, 1, 1, 1, 1, 2, 3, 4};

	/******************** examples for Euclidean (L2) Metric **************************/
	std::cout << "Euclidean (L2) Metric" << std::endl;
	mtrc::Euclidean<double> EuclideanL2Distance;
	auto startTime_1 = std::chrono::steady_clock::now();
	auto result_1 = EuclideanL2Distance(v0, v1);
	auto endTime_1 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_1 << " (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_1 - startTime_1).count()) / 1000
			  << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out:
	// Euclidean (L2) Metric
	// result: 2 (Time = 0.018 ms)

	/******************** examples for Euclidean Threshold **************************/
	std::cout << "Euclidean Threshold" << std::endl;
	mtrc::Euclidean_thresholded<double> EuclideanThresholdDistance(1000.0, 3000.0);
	auto startTime_2 = std::chrono::steady_clock::now();
	auto result_2 = EuclideanThresholdDistance(v0, v1);
	auto endTime_2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_2 << " (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_2 - startTime_2).count()) / 1000
			  << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out:
	// Euclidean Threshold
	// result: 1000 (Time = 0.008 ms)

	/******************** examples for Manhattan/Cityblock (L1) Metric **************************/
	std::cout << "Manhattan/Cityblock (L1) Metric" << std::endl;
	mtrc::Manhattan<double> manhattanL1Distance;
	auto startTime_3 = std::chrono::steady_clock::now();
	auto result_3 = manhattanL1Distance(v0, v1);
	auto endTime_3 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_3 << " (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_3 - startTime_3).count()) / 1000
			  << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out:
	// Manhattan/Cityblock (L1) Metric
	// result: 4 (Time = 0.007 ms)

	/******************** examples for Minkowski (L general) Metric **************************/
	std::cout << "Minkowski (L general) Metric" << std::endl;
	mtrc::P_norm<double> pNormDistance(2);
	auto startTime_4 = std::chrono::steady_clock::now();
	auto result_4 = pNormDistance(v0, v1);
	auto endTime_4 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_4 << " (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_4 - startTime_4).count()) / 1000
			  << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out:
	// Minkowski (L general) Metric
	// result: 2 (Time = 0.061 ms)

	/******************** examples for Cosine Metric **************************/
	std::cout << "Cosine Metric" << std::endl;
	// mtrc::distance::Cosine<double> cosineDistance;
	auto cosineDistance =
		mtrc::Cosine<double>(); // default return type is double, so we do not need to set <double> explicitely
	auto startTime_5 = std::chrono::steady_clock::now();
	auto result_5 = cosineDistance(v0, v1);
	auto endTime_5 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_5 << " (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_5 - startTime_5).count()) / 1000
			  << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out:
	// Cosine Metric
	// result: 0.970143 (Time = 0.038 ms)

	/************* examples for Euclidean Metric with Numeric input ***************/
	std::cout << "Numeric overload of Euclidean Metric" << std::endl;
	// mtrc::numeric::DynamicVector<double> vb0; // this will not compile!!
	// mtrc::numeric::DynamicVector<double> vb1;
	mtrc::numeric::CompressedVector<double> vb0 = {0, 1, 1, 1, 1, 1, 2, 3}; // OK
	mtrc::numeric::CompressedVector<double> vb1 = {1, 1, 1, 1, 1, 2, 3, 4};
	// mtrc::numeric::CompressedMatrix<double> vb0 = { {0, 1, 1, 1}, {1, 1, 2, 3} }; // OK
	// mtrc::numeric::CompressedMatrix<double> vb1 = { {1, 1, 1, 1}, {1, 2, 3, 4} };
	//  we reuse mtrc::Euclidean<double> EuclideanL2Distance;
	auto startTime_6 = std::chrono::steady_clock::now();
	auto result_6 = EuclideanL2Distance(vb0, vb1);
	auto endTime_6 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_6 << " (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_6 - startTime_6).count()) / 1000
			  << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out:
	// Numeric overload of Euclidean Metric
	// result: 2 (Time = 0.036 ms)

	return 0;
}
