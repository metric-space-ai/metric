#include <vector>
#include <iostream>
#include <chrono>
#include "../metric_distance.cpp"


int main()
{
	/******************** examples for Edit distance (for strings) **************************/
	// example for string

	std::cout << "we have started" << std::endl;
	std::cout << "" << std::endl;

	std::string str1 = "1011001100110011001111111";
	std::string str2 = "1000011001100110011011100";
	std::string str3 = "Absolutly different string";

	metric::distance::Edit<std::string> distance;

	auto t1 = std::chrono::steady_clock::now();
	auto result1 = distance(str1, str2);
	auto t2 = std::chrono::steady_clock::now();
	std::cout << "result for close strings: " << result1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;

	auto result2 = distance(str1, str3);
	auto t3 = std::chrono::steady_clock::now();
	std::cout << "result for different strings: " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	
	return 0;
}
