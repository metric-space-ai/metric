#include <vector>
#include <iostream>
#include <chrono>
#include "../metric_distance.cpp"


int main()
{
	/******************** examples for Time Warp Elastic Distance (for curves, series or time-series) **************************/
	// example for curve

	std::cout << "we have started" << std::endl;
	std::cout << "" << std::endl;

	/*** here are some data records ***/
	std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
	std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
	std::vector<double> v2 = { 2, 2, 2, 2, 2, 2, 2, 2 };

	metric::distance::TWED<double> distance;

	auto t1 = std::chrono::steady_clock::now();
	auto result1 = distance(v0, v1);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result for a close curves (series): " << result1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;

	auto result2 = distance(v0, v2);
	auto t3 = std::chrono::steady_clock::now();
	std::cout << "result for a far curves (series): " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;


	return 0;
}
