/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#include <algorithm>
#include <limits>
#include <tuple>
#include <vector>

namespace metric {

inline std::tuple<int, int> getMinPosition(std::vector<std::vector<double>> &distanceMatrix)
{
	int minX = 0;
	int minY = 1;
	double dist = std::numeric_limits<double>::max();

	for (size_t i = 0; i < distanceMatrix.size(); i++) {
		for (size_t j = i + 1; j < distanceMatrix.size(); j++) {
			if (distanceMatrix[i][j] < dist) {
				dist = distanceMatrix[i][j];
				minX = i;
				minY = j;
			}
		}
	}

	return std::make_tuple(minX, minY);
}

template <typename T> std::pair<bool, int> findInVector(const std::vector<T> &vecOfElements, const T &element)
{
	std::pair<bool, int> result;

	// Find given element in vector
	auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

	if (it != vecOfElements.end()) {
		result.second = distance(vecOfElements.begin(), it);
		result.first = true;
	} else {
		result.first = false;
		result.second = -1;
	}

	return result;
}

template <typename T, typename Distance> void HierarchicalClustering<T, Distance>::initialize()
{
	for (size_t i = 0; i < sourceData.size(); i++) {
		auto cluster = Cluster<T>({sourceData[i]});
		clusters.push_back(cluster);
	}
}

template <typename T, typename Distance>
std::vector<std::vector<double>> HierarchicalClustering<T, Distance>::calculateDistances()
{
	std::vector<std::vector<double>> distanceMatrix(clusters.size(), std::vector<double>(clusters.size()));
	Distance distancer;

	for (size_t i = 0; i < clusters.size(); i++) {
		for (size_t j = 0; j < clusters.size(); j++) {
			distanceMatrix[i][j] = distancer(clusters[i].centroid, clusters[j].centroid);
		}
	}

	return distanceMatrix;
}

template <typename T, typename Distance> void HierarchicalClustering<T, Distance>::hierarchical_clustering()
{
	initialize();

	std::vector<std::vector<double>> distanceMatrix;
	std::vector<Cluster<T>> newClusters;
	std::vector<T> newData;
	std::vector<int> mergedIndexes;
	int closest;
	std::pair<bool, int> loockupResultX;
	std::pair<bool, int> loockupResultY;

	int x;
	int y;

	while (clusters.size() > clustersNum) {
		distanceMatrix = calculateDistances();
		newClusters.clear();
		mergedIndexes.clear();

		while ((int)mergedIndexes.size() / 2 < (int)clusters.size() / 2) {
			std::tie(x, y) = getMinPosition(distanceMatrix);
			distanceMatrix[x][y] = std::numeric_limits<double>::max();

			loockupResultX = findInVector<int>(mergedIndexes, x);
			loockupResultY = findInVector<int>(mergedIndexes, y);
			if (!loockupResultX.first && !loockupResultY.first) {
				mergedIndexes.push_back(x);
				mergedIndexes.push_back(y);
				newData.clear();
				for (size_t i = 0; i < clusters[x].data.size(); i++) {
					newData.push_back(clusters[x].data[i]);
				}
				for (size_t i = 0; i < clusters[y].data.size(); i++) {
					newData.push_back(clusters[y].data[i]);
				}

				auto cluster = Cluster<T>(newData);
				newClusters.push_back(cluster);
			}
		}

		// get the last element
		if (mergedIndexes.size() < clusters.size()) {

			for (size_t i = 0; i < clusters.size(); i++) {
				loockupResultX = findInVector<int>(mergedIndexes, i);
				if (!loockupResultX.first) {
					mergedIndexes.push_back(i);
					newData.clear();
					for (size_t j = 0; j < clusters[i].data.size(); j++) {
						newData.push_back(clusters[i].data[j]);
					}

					auto cluster = Cluster<T>(newData);
					newClusters.push_back(cluster);
				}
			}
		}
		clusters = newClusters;
	}
}

} // namespace metric
