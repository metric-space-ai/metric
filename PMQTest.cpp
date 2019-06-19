#include <iostream>
#include "modules/crossfilter.hpp"
#include "utils/poor_mans_quantum.hpp"

int main()
{
	std::cout << "we have started" << std::endl;
	std::cout << '\n';


	std::vector<double> datasetColumn_0;
	std::vector<double> datasetColumn_1;
	for (auto i = 0; i < 100; ++i)
	{
		datasetColumn_0.push_back(0);
	}
	for (auto i = 0; i < 100; ++i)
	{
		datasetColumn_1.push_back(1);
	}

	PMQ<Discrete<float>> set0(datasetColumn_0);
	PMQ<Discrete<float>> set1(datasetColumn_1);
	float significantDifferent = (set1 != set0); // values between 0...1

	return 0;
}