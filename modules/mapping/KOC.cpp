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
			const std::vector<recType>& samples, int num_clusters, int min_cluster_size)
		{
			som_.train(samples);
			calculate_std_deviations_for_nodes(samples, samples.size());
			std::tie(clusters, centroids, clusters_counts) = clusterize_nodes(num_clusters, min_cluster_size);
		}


		//template <class recType, class Graph, class Metric, class Distribution>
		//void KOC<recType, Graph, Metric, Distribution>::estimate(const std::vector<recType>& samples, const size_t sampleSize, int num_clusters, int min_cluster_size)
		//{
		//	som_.estimate(samples, sampleSize);
		//	calculate_std_deviations_for_nodes(samples, sampleSize);
		//	std::tie(clusters, centroids, clusters_counts) = clusterize_nodes(num_clusters, min_cluster_size);
		//}


		template <class recType, class Graph, class Metric, class Distribution>
		std::vector<bool> KOC<recType, Graph, Metric, Distribution>::check_if_anomaly(const std::vector<recType>& samples)
		{
			std::vector<bool> result;

			for (size_t i = 0; i < samples.size(); i++)
			{
				// if entropy less then min entropy level then it is anomaly
				result.push_back(check_if_anomaly(samples[i]));
			}

			return result;
		}

		template <class recType, class Graph, class Metric, class Distribution>
		bool KOC<recType, Graph, Metric, Distribution>::check_if_anomaly(const recType& sample)
		{
			auto reduced = som_.encode(sample);
			auto bmu = som_.BMU(sample);
			// if closest distance more then max closest distance level then it is anomaly
			return reduced[bmu] > nodes_std_deviations[bmu] * anomaly_sigma_;
		}

		
		template <class recType, class Graph, class Metric, class Distribution>
		std::vector<int> KOC<recType, Graph, Metric, Distribution>::assign_to_clusters(
			const std::vector<recType>& samples)
		{				
			std::vector<int> assignments;
			auto anomalies = check_if_anomaly(samples);
			for (size_t i = 0; i < samples.size(); i++)
			{
				// we want 0 label as anomaly
				if (anomalies[i])
				{
					assignments.push_back(0);
				}
				else
				{
					auto bmu = som_.BMU(samples[i]);
					assignments.push_back(clusters[bmu]);
				}
			}

			return assignments;
		};


		template <class recType, class Graph, class Metric, class Distribution>
		std::tuple<std::vector<size_t>, std::vector<typename recType::value_type>>
			KOC<recType, Graph, Metric, Distribution>::top_outliers(const std::vector<recType>& samples, int count)
		{
			auto assignments = assign_to_clusters(samples);

			std::vector<T> distances;
			for (int i = 0; i < samples.size(); i++)
			{
				auto reduced = som_.encode(samples[i]);
				auto bmu = som_.BMU(samples[i]);
				distances.push_back(reduced[bmu] - nodes_std_deviations[bmu] * anomaly_sigma_);
			}

			auto idxs = sort_indexes(distances);
			std::vector<T> sorted_distances;
			//std::vector<int> sorted_assignments;
			if (idxs.size() > count)
			{
				idxs.resize(count);
			}
			for (int i = 0; i < idxs.size(); i++)
			{
				sorted_distances.push_back(distances[idxs[i]]);
				//sorted_assignments.push_back(assignments[idxs[i]]);
			}

			return {idxs, sorted_distances};
		};


		/// PRIVATE

		template <class recType, class Graph, class Metric, class Distribution>
		void KOC<recType, Graph, Metric, Distribution>::calculate_std_deviations_for_nodes(const std::vector<recType>& samples, int sampleSize)
		{		
			if (sampleSize > samples.size())
			{
				sampleSize = samples.size();
			}
			// Random samples 
			std::vector<size_t> randomized_samples(samples.size());
			std::iota(randomized_samples.begin(), randomized_samples.end(), 0);

			// shuffle samples after all was processed		

			std::default_random_engine random_generator(random_seed_);

			std::shuffle(randomized_samples.begin(), randomized_samples.end(), random_generator);
	
			int num_nodes = som_.getNodesNumber();

			std::vector<int> closest_distances(num_nodes, 0);
			std::vector<T> square_distances_sum(num_nodes, 0);
			for (size_t i = 0; i < sampleSize; i++)
			{
				size_t sample_idx = randomized_samples[i];
		
				auto sample = next(samples.begin(), sample_idx);
				auto reduced = som_.encode(*sample);
				auto bmu = som_.BMU(*sample);

				square_distances_sum[bmu] += reduced[bmu] * reduced[bmu];
				closest_distances[bmu]++;
			}

			nodes_std_deviations = std::vector<T>(num_nodes);
			for (size_t i = 0; i < num_nodes; i++)
			{
				if (closest_distances[i] > 0)
				{
					nodes_std_deviations[i] = sqrt(square_distances_sum[i] / closest_distances[i]);
				}
				else
				{
					nodes_std_deviations[i] = 0;
				}
			}
		}

		template <class recType, class Graph, class Metric, class Distribution>
		std::tuple<std::vector<int>, std::vector<recType>, std::vector<int>> KOC<recType, Graph, Metric, Distribution>::clusterize_nodes(int num_clusters, int min_cluster_size)
		{
			int current_min_cluster_size = -1;

			auto nodes_data = som_.get_weights();

			std::string metric_name = "euclidian";
			if (typeid(Metric) == typeid(metric::CosineInverted<typename Metric::value_type>))
			{
				metric_name = "cosine_inverted";
			}
			else if (typeid(Metric) == typeid(metric::Manhatten<typename Metric::value_type>))
			{
				metric_name = "manhatten";
			}

			if (min_cluster_size > nodes_data.size())
			{
				min_cluster_size = nodes_data.size();
			}

			while (current_min_cluster_size < min_cluster_size)
			{
				// clustering on the reduced data
				
				auto [assignments, exemplars, counts] = metric::kmeans(nodes_data, num_clusters, iterations_, metric_name, random_seed_);

				std::vector<int>::iterator result = std::min_element(counts.begin(), counts.end());
				current_min_cluster_size = counts[std::distance(counts.begin(), result)];

				int num_ones = 0;
				for (auto i : counts)
				{
					if (i <= 1)
					{
						num_ones++;
					}
				}

				int new_num_clusters = num_clusters * std::min(0.5, 0.1 * num_ones);
				if (new_num_clusters == num_clusters)
				{
					new_num_clusters--;
				}
				if (new_num_clusters <= 0)
				{
					new_num_clusters = 1;
				}

				num_clusters = new_num_clusters;

				if (current_min_cluster_size >= min_cluster_size)
				{
					for (size_t i = 0; i < assignments.size(); i++)
					{
						// we want 0 label as anomaly, so increment original labels
						assignments[i]++;
					}

					return { assignments, exemplars, counts };
				}
			}

			return { std::vector<int>(), std::vector<recType>(), std::vector<int>() };
		}

		template <class recType, class Graph, class Metric, class Distribution>
		template <typename T1>
		std::vector<size_t> KOC<recType, Graph, Metric, Distribution>::sort_indexes(const std::vector<T1> &v)
		{
			// initialize original index locations
			std::vector<size_t> idx(v.size());
			std::iota(idx.begin(), idx.end(), 0);

			// sort indexes based on comparing values in v
			std::sort(idx.begin(), idx.end(),
				[&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

			return idx;
		}

	}  // namespace KOC_details

	//
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesNumber, double anomaly_sigma,
		double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max) : 
		graph_(nodesNumber), 
		metric_(), 
		distribution_(distribution_min, distribution_max),
		anomaly_sigma_(anomaly_sigma),
		start_learn_rate_(start_learn_rate),
		finish_learn_rate_(finish_learn_rate),
		iterations_(iterations),
		random_seed_(std::chrono::system_clock::now().time_since_epoch().count()),
		neighborhood_start_size_(std::sqrt(double(nodesNumber))),
		neigbour_range_decay_(2.0)
	{

	}
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesWidth, size_t nodesHeight, double anomaly_sigma,
		double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max) : 
		graph_(nodesWidth, nodesHeight), 
		metric_(), 
		distribution_(distribution_min, distribution_max),
		anomaly_sigma_(anomaly_sigma),
		start_learn_rate_(start_learn_rate),
		finish_learn_rate_(finish_learn_rate),
		iterations_(iterations),
		random_seed_(std::chrono::system_clock::now().time_since_epoch().count()),
		neighborhood_start_size_(std::sqrt(double(nodesWidth * nodesHeight))),
		neigbour_range_decay_(2.0)
	{
	}
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesNumber, double anomaly_sigma,
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed) : 
		graph_(nodesNumber), 
		metric_(), 
		distribution_(distribution_min, distribution_max),
		anomaly_sigma_(anomaly_sigma),
		start_learn_rate_(start_learn_rate),
		finish_learn_rate_(finish_learn_rate),
		iterations_(iterations),
		random_seed_(random_seed),
		neighborhood_start_size_(neighborhood_start_size),
		neigbour_range_decay_(neigbour_range_decay)
	{
	}
	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_factory<recType, Graph, Metric, Distribution>::KOC_factory(size_t nodesWidth, size_t nodesHeight, double anomaly_sigma,
			double start_learn_rate, double finish_learn_rate, size_t iterations, T distribution_min, T distribution_max, 
			double neighborhood_start_size, double neigbour_range_decay, long long random_seed) : 
		graph_(nodesWidth, nodesHeight), 
		metric_(), 
		distribution_(distribution_min, distribution_max),
		anomaly_sigma_(anomaly_sigma),
		start_learn_rate_(start_learn_rate),
		finish_learn_rate_(finish_learn_rate),
		iterations_(iterations),
		random_seed_(random_seed),
		neighborhood_start_size_(neighborhood_start_size),
		neigbour_range_decay_(neigbour_range_decay)
	{
	}

	
	template <class recType, class Graph, class Metric, class Distribution>
	KOC_details::KOC<recType, Graph, Metric, Distribution> KOC_factory<recType, Graph, Metric, Distribution>::operator()(const std::vector<recType>& samples, int num_clusters, int min_cluster_size)
	{
		KOC_details::KOC<recType, Graph, Metric, Distribution> koc(graph_, metric_, anomaly_sigma_, start_learn_rate_, finish_learn_rate_, iterations_, distribution_,
			neighborhood_start_size_, neigbour_range_decay_, random_seed_);

		koc.train(samples, num_clusters, min_cluster_size);

		return koc;
	}

}  // namespace metric



#endif