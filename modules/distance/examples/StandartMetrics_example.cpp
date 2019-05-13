#include <vector>
#include <iostream>
#include <chrono>
#include "../metric_distance.cpp"


int main()
{
	std::cout << "we have started" << std::endl;
	std::cout << "" << std::endl;

	/*** here are some data records ***/
	std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
	std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };


	/******************** examples for Euclidian (L2) Metric **************************/
	std::cout << "Euclidian (L2) Metric" << std::endl;
	metric::distance::Euclidian<double> euclidianL2Distance;
	auto startTime_1 = std::chrono::steady_clock::now();
	auto result_1 = euclidianL2Distance(v0, v1);
	auto endTime_1 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_1 - startTime_1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out: 
	// Euclidian (L2) Metric
	// result: 2 (Time = 0.018 ms)

	/******************** examples for Euclidian Threshold **************************/
	std::cout << "Euclidian Threshold" << std::endl;
	metric::distance::Euclidian_thresholded<double> euclidianThresholdDistance(1000.0, 3000.0);
	auto startTime_2 = std::chrono::steady_clock::now();
	auto result_2 = euclidianThresholdDistance(v0, v1);
	auto endTime_2 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_2 - startTime_2).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out: 
	// Euclidian Threshold
	// result: 1000 (Time = 0.008 ms)

	/******************** examples for Manhatten/Cityblock (L1) Metric **************************/
	std::cout << "Manhatten/Cityblock (L1) Metric" << std::endl;
	metric::distance::Manhatten<double> manhattenL1Distance;
	auto startTime_3 = std::chrono::steady_clock::now();
	auto result_3 = manhattenL1Distance(v0, v1);
	auto endTime_3 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_3 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_3 - startTime_3).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out: 
	// Manhatten/Cityblock (L1) Metric
	// result: 4 (Time = 0.007 ms)

	/******************** examples for Minkowski (L general) Metric **************************/
	std::cout << "Minkowski (L general) Metric" << std::endl;
	metric::distance::P_norm<double> pNormDistance(2);
	auto startTime_4 = std::chrono::steady_clock::now();
	auto result_4 = pNormDistance(v0, v1);
	auto endTime_4 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_4 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_4 - startTime_4).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out: 
	// Minkowski (L general) Metric
	// result: 2 (Time = 0.061 ms)

	/******************** examples for Cosine Metric **************************/
	std::cout << "Cosine Metric" << std::endl;
	metric::distance::Cosine<double> cosineDistance;
	auto startTime_5 = std::chrono::steady_clock::now();
	auto result_5 = cosineDistance(v0, v1);
	auto endTime_5 = std::chrono::steady_clock::now();
	std::cout << "result: " << result_5 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime_5 - startTime_5).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	// out: 
	// Cosine Metric
	// result: 0.970143 (Time = 0.038 ms)

	return 0;
}
