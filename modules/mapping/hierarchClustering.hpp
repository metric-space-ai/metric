/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#include <vector>

namespace metric
{

	std::tuple <int, int> getMinPosition(std::vector<std::vector<double>> &distanceMatrix);

	template <typename T>
	std::pair<bool, int> findInVector(const std::vector<T>  & vecOfElements, const T  & element);

	///////////////////////

	template <typename T>
	class Cluster {
	public:
		std::vector<T> data;
		T centroid;

		Cluster(const std::vector<T> &d)
		{
			data = d;
			calculateCentroid(data);
		}

		template<typename T>
		void calculateCentroid(const std::vector<T> &d) {
			centroid = T(data[0].size());
			typename T::value_type sum;
			for (size_t i = 0; i < data[0].size(); i++)
			{
				sum = 0;
				for (size_t j = 0; j < data.size(); j++)
				{
					sum += data[j][i];
				}
				centroid[i] = (typename T::value_type) sum / data.size();
			}
		}

		template<>
		void calculateCentroid(const std::vector<std::string> &d) {
			centroid = data[0];
		}
	};
	
	template <typename T, typename Metric>
	class HierarchicalClustering {
	private:

		std::vector<std::vector<double>> calculateDistances();

	protected:

	public:
		std::vector<Cluster<T>> clusters;
		std::vector<T> sourceData;
		int clustersNum;

		HierarchicalClustering(
			const std::vector<T> &data,
			const int &k)
		{
			sourceData = data;
			clustersNum = k;
		}

		void initialize();

		void hierarchical_clustering();

	}; // class HierarchicalClustering


} // namespace clustering

#include "hierarchClustering.cpp"