/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_MAPPING_KOC_CPP
#define _METRIC_MAPPING_KOC_CPP
#include "KOC.hpp"
namespace metric {
	
	namespace KOC_details {
		template <class recType, class Graph, class Metric, class Distribution>
		void KOC<recType, Graph, Metric, Distribution>::train(
			const std::vector<std::vector<T>>& samples, int num_clusters)
		{
			SOM<recType, Graph, Metric, Distribution>::train(samples);
			parse_distances(samples, samples.size());
			std::tie(clusters, centroids, clusters_counts) = clusterize_nodes(num_clusters);
		}


		template <class recType, class Graph, class Metric, class Distribution>
		void KOC<recType, Graph, Metric, Distribution>::estimate(const std::vector<std::vector<T>>& samples, const size_t sampleSize, int num_clusters)
		{
			SOM<recType, Graph, Metric, Distribution>::estimate(samples, sampleSize);
			parse_distances(samples, sampleSize);
			std::tie(clusters, centroids, clusters_counts) = clusterize_nodes(num_clusters);
		}

		template <class recType, class Graph, class Metric, class Distribution>
		std::vector<double> KOC<recType, Graph, Metric, Distribution>::encode(const recType& sample)
		{
			return SOM<recType, Graph, Metric, Distribution>::encode(sample);
		}

		template <class recType, class Graph, class Metric, class Distribution>
		std::vector<int> KOC<recType, Graph, Metric, Distribution>::encode(const std::vector<std::vector<T>>& samples, double anomaly_threshold)
		{
			std::vector<int> result;

			for (size_t i = 0; i < samples.size(); i++)
			{
				if (check_if_anomaly(samples[i], anomaly_threshold))
				{
					result.push_back(0);
				}
				else
				{
					//auto reduced = SOM<recType, Graph, Metric, Distribution>::encode(samples[i]);
					auto bmu = SOM<recType, Graph, Metric, Distribution>::BMU(samples[i]);
					result.push_back(clusters[bmu]);
				}
			}

			return result;
		}

		template <class recType, class Graph, class Metric, class Distribution>
		size_t KOC<recType, Graph, Metric, Distribution>::BMU(const recType& sample) const
		{
			return SOM<recType, Graph, Metric, Distribution>::BMU(sample);
		}

		template <class recType, class Graph, class Metric, class Distribution>
		std::vector<bool> KOC<recType, Graph, Metric, Distribution>::check_if_anomaly(const std::vector<recType>& samples, double anomaly_threshold)
		{
			std::vector<bool> result;
	
			auto entropy_range = reduced_mean_entropy - (reduced_mean_entropy - reduced_min_entropy) * (1 + anomaly_threshold);
			std::cout << "entropy_range: " << entropy_range << 
				" reduced_mean_entropy: " << reduced_mean_entropy << " reduced_min_entropy: " << reduced_min_entropy << " reduced_max_entropy: " << reduced_max_entropy << std::endl;

			for (size_t i = 0; i < samples.size(); i++)
			{
				// if entropy less then min entropy level then it is anomaly
				result.push_back(check_if_anomaly(samples[i], anomaly_threshold));
			}

			return result;
		}

		template <class recType, class Graph, class Metric, class Distribution>
		bool KOC<recType, Graph, Metric, Distribution>::check_if_anomaly(const recType& sample, double anomaly_threshold)
		{
			//auto closest_distance_range = reduced_mean_closest_distance - (reduced_mean_closest_distance - reduced_min_closest_distance) * (1 + anomaly_threshold_);
			auto entropy_range = reduced_mean_entropy - (reduced_mean_entropy - reduced_min_entropy) * (1 + anomaly_threshold);

			auto reduced = SOM<recType, Graph, Metric, Distribution>::encode(sample);
			auto bmu = SOM<recType, Graph, Metric, Distribution>::BMU(sample);
			// if closest distance more then max closest distance level then it is anomaly
			// return reduced[bmu] > closest_distance_range;
		
			std::sort(reduced.begin(), reduced.end());	

			std::vector<std::vector<T>> reduced_reshaped;
			for (size_t j = 0; j < reduced.size(); j++)
			{
				reduced_reshaped.push_back({reduced[j]});
			}
			auto e = entropy(reduced_reshaped, 3, 2.0, SOM<recType, Graph, Metric, Distribution>::metric);
			
			//std::cout << e << "entropy_range:" << entropy_range << std::endl;
			// if entropy less then min entropy level then it is anomaly
			return e < entropy_range;
		}

		
		template <class recType, class Graph, class Metric, class Distribution>
		std::vector<int> KOC<recType, Graph, Metric, Distribution>::result(
			const std::vector<std::vector<T>>& samples, double anomaly_threshold = 0.0)
		{				
			std::vector<int> assignments;
			auto anomalies = check_if_anomaly(samples, anomaly_threshold);						
			for (size_t i = 0; i < samples.size(); i++)
			{
				// we want 0 label for anomalies
				if (anomalies[i])
				{
					assignments.push_back(0);
				}
				else
				{
					auto bmu = BMU(samples[i]);
					assignments.push_back(clusters[bmu]);
				}
			}

			return assignments;
		};

		// PRIVATE

		template <class recType, class Graph, class Metric, class Distribution>
		void KOC<recType, Graph, Metric, Distribution>::parse_distances(const std::vector<std::vector<T>>& samples, int sampleSize)
		{		
			if (sampleSize > samples.size())
			{
				sampleSize = samples.size();
			}
			// Random samples 
			std::vector<size_t> randomized_samples(samples.size());
			std::iota(randomized_samples.begin(), randomized_samples.end(), 0);

			// shuffle samples after all was processed		
			std::shuffle(randomized_samples.begin(), randomized_samples.end(), std::mt19937 { std::random_device {}() });
	
			std::vector<T> entropies;
			std::vector<T> closest_distances;  // closest distances to the nodes for each sample from train dataset
			T entropies_sum = 0;
			T distances_sum = 0;
			for (size_t i = 0; i < sampleSize; i++)
			{
				size_t sample_idx = randomized_samples[i];
		
				auto reduced = SOM<recType, Graph, Metric, Distribution>::encode(samples[sample_idx]);
				auto bmu = SOM<recType, Graph, Metric, Distribution>::BMU(samples[sample_idx]);
				distances_sum += reduced[bmu];
				closest_distances.push_back(reduced[bmu]);
		
				std::sort(reduced.begin(), reduced.end());	

				std::vector<recType> reduced_reshaped;
				for (size_t j = 0; j < reduced.size(); j++)
				{
					reduced_reshaped.push_back({reduced[j]});
				}
				auto e = entropy(reduced_reshaped, 3, 2.0, SOM<recType, Graph, Metric, Distribution>::metric);
				entropies_sum += e;
				entropies.push_back(e);
			}

			reduced_mean_entropy = entropies_sum / entropies.size();
			auto result = std::max_element(entropies.begin(), entropies.end());
			reduced_max_entropy = entropies[std::distance(entropies.begin(), result)];
			result = std::min_element(entropies.begin(), entropies.end());
			reduced_min_entropy = entropies[std::distance(entropies.begin(), result)];

			reduced_mean_closest_distance = distances_sum / closest_distances.size();
			result = std::max_element(closest_distances.begin(), closest_distances.end());
			reduced_max_closest_distance = closest_distances[std::distance(closest_distances.begin(), result)];
			result = std::min_element(closest_distances.begin(), closest_distances.end());
			reduced_min_closest_distance = closest_distances[std::distance(closest_distances.begin(), result)];
		}

		template <class recType, class Graph, class Metric, class Distribution>
		std::tuple<std::vector<int>, std::vector<std::vector<recType::value_type>>, std::vector<int>> KOC<recType, Graph, Metric, Distribution>::clusterize_nodes(int num_clusters)
		{
			int min_cluster_size = 0;
			//int num_clusters = KOC<recType, Graph, Metric, Distribution>::getNodesNumber();

			auto nodes_data = KOC<recType, Graph, Metric, Distribution>::get_weights();

			while (min_cluster_size <= 1)
			{
				// clustering on the reduced data
				
				auto [assignments, exemplars, counts] = metric::kmeans(nodes_data, num_clusters, 1000);
				//metric::Matrix<recType, metric::Euclidian<double>, double> distance_matrix(nodes_data);
				//auto [assignments_size_t, exemplars_unknwn, counts_size_t] = metric::dbscan(distance_matrix, (double)0.7, 1);
				//std::vector<std::vector<recType::value_type>> exemplars;
				//std::vector<int> assignments;
				//for (const auto i : assignments_size_t)
				//{
				//	assignments.push_back(static_cast<int>(i));
				//}
				//std::vector<int> counts;
				//for (const auto i : counts_size_t)
				//{
				//	counts.push_back(static_cast<int>(i));
				//}
		
				std::vector<int>::iterator result = std::min_element(counts.begin(), counts.end());
				min_cluster_size = counts[std::distance(counts.begin(), result)];	

				int num_ones = 0;
				for (auto i : counts)
				{
					if (i <= 1)
					{
						num_ones++;
					}
				}

				int new_num_clusters = num_clusters * min(0.5, 0.1 * num_ones);
				if (new_num_clusters == num_clusters)
				{
					new_num_clusters--;
				}

				num_clusters = new_num_clusters;

				if (num_clusters <= 1)
				{
					for (size_t i = 0; i < assignments.size(); i++)
					{
						// we want 0 label for anomalies, so increment original labels
						assignments[i]++;
					}

					return { assignments, exemplars, counts };
				}
			}

			return { std::vector<int>(), std::vector<std::vector<recType::value_type>>(), std::vector<int>() };
		}

	}  // namespace KOC_details

	//
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesNumber, 
		double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1) : 
		graph_(nodesNumber), 
		metric_(), 
		distribution_(distribution_min, distribution_max)
	{
		start_learn_rate_ = start_learn_rate;
		finish_learn_rate_ = finish_learn_rate;
		iterations_ = iterations;
	
		random_seed_ = std::chrono::system_clock::now().time_since_epoch().count();
	
		neighborhood_start_size_ = std::sqrt(double(nodesNumber));
		neigbour_range_decay_ = 2.0;
	}
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesWidth = 5, size_t nodesHeight = 4, 
		double start_learn_rate = 0.8, double finish_learn_rate = 0.0, size_t iterations = 20, T distribution_min = -1, T distribution_max = 1) : 
		graph_(nodesWidth, nodesHeight), 
		metric_(), 
		distribution_(distribution_min, distribution_max)
	{
		start_learn_rate_ = start_learn_rate;
		finish_learn_rate_ = finish_learn_rate;
		iterations_ = iterations;
	
		random_seed_ = std::chrono::system_clock::now().time_since_epoch().count();
	
		neighborhood_start_size_ = std::sqrt(double(nodesWidth * nodesHeight));
		neigbour_range_decay_ = 2.0;
	}
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesNumber, 
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed) : 
		graph_(nodesNumber), 
		metric_(), 
		distribution_(distribution_min, distribution_max)
	{
		start_learn_rate_ = start_learn_rate;
		finish_learn_rate_ = finish_learn_rate;
		iterations_ = iterations;
	
		random_seed_ = random_seed;
	
		neighborhood_start_size_ = neighborhood_start_size;
		neigbour_range_decay_ = neigbour_range_decay;
	}
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesWidth, size_t nodesHeight, 
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed) : 
		graph_(nodesWidth, nodesHeight), 
		metric_(), 
		distribution_(distribution_min, distribution_max)
	{
		start_learn_rate_ = start_learn_rate;
		finish_learn_rate_ = finish_learn_rate;
		iterations_ = iterations;
	
		random_seed_ = random_seed;
	
		neighborhood_start_size_ = neighborhood_start_size;
		neigbour_range_decay_ = neigbour_range_decay;
	}

	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_details::KOC<recType, Graph, Metric, Distribution> KOC_factory<recType, Graph, Metric, Distribution>::operator()(const std::vector<std::vector<T>>& samples, int num_clusters)
	{
		KOC_details::KOC<recType, Graph, Metric, Distribution> koc(graph_, metric_, start_learn_rate_, finish_learn_rate_, iterations_, distribution_, 
			neighborhood_start_size_, neigbour_range_decay_, random_seed_);

		koc.train(samples, num_clusters);

		return koc;
	}

}  // namespace metric



#endif