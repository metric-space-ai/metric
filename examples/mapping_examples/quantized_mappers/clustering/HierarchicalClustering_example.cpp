/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "modules/mapping.hpp"
#include "modules/distance.hpp"



int main()
{
	std::cout << "Hierarchical Clustering example have started" << std::endl;
	std::cout << '\n';


	std::cout << "Data as records (vector of floats)" << std::endl;
	std::cout << '\n';

	using Record =  std::vector<float>;

	std::vector<Record> data{
		   {0, 0, 0, 0, 0},
		   {1, 1, 1, 1, 1},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000}
	};
	   	 
	auto hc = metric::HierarchicalClustering<Record, metric::Euclidean<Record::value_type>>(data, 3);
	hc.hierarchical_clustering();

	for (size_t i = 0; i < hc.clusters.size(); i++)
	{
		std::cout << "cluster #" << i << std::endl;
		for (size_t j = 0; j < hc.clusters[i].data.size(); j++)
		{
			for (size_t k = 0; k < hc.clusters[i].data[j].size(); k++)
			{
				std::cout << hc.clusters[i].data[j][k] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}


	// clustering on strings
	
	std::cout << "Data as strings" << std::endl;
	std::cout << '\n';

	std::vector<std::string> str_data{
		   "1011001100110011001111111",
		   "1000011001100110011011100",
		   "Absolutly different string 1",
		   "Absolutly different string 2",
		   "Test string 1",
		   "Test string 2",
		   "Test string 3"
	};

	auto hc2 = metric::HierarchicalClustering<std::string, metric::Edit<std::string>>(str_data, 4);
	hc2.hierarchical_clustering();

	for (size_t i = 0; i < hc2.clusters.size(); i++)
	{
		std::cout << "cluster #" << i << std::endl;
		for (size_t j = 0; j < hc2.clusters[i].data.size(); j++)
		{
			std::cout << hc2.clusters[i].data[j] << std::endl; 
		}
		std::cout << std::endl;
	}

	return 0;

}
