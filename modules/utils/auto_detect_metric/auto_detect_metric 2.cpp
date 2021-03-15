/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 PANDA Team
*/

namespace metric {

	MetricAutoDetector::MetricAutoDetector()
	{
	}

	
	template <typename Record, typename Graph>
	std::string MetricAutoDetector::detect(Graph &graph, int graph_w, int graph_h, std::vector<Record> dataset, bool isEstimate)
	{
		std::vector<std::string> metric_type_names = {"Euclidean", "Manhatten", "P_norm", "Euclidean_thresholded", "Cosine", "Chebyshev", 
			"Earth Mover Distance", "SSIM", "TWED"};

		// Random updating 
		std::vector<size_t> randomized_indexes(dataset.size());
		std::iota(randomized_indexes.begin(), randomized_indexes.end(), 0);
		// shuffle samples after all was processed		
		std::shuffle(randomized_indexes.begin(), randomized_indexes.end(), std::mt19937{ std::random_device {}() });
		
		double relative_diff_mean;
		std::vector<double> relative_diff_means;
		for (auto metric_type : metric_type_names)
		{
			if (metric_type == "Euclidean")
			{
				// Euclidean
				metric::Euclidean<double> distance;
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::Euclidean<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "Manhatten")
			{
				// Manhatten
				metric::Manhatten<double> distance;
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::Manhatten<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "P_norm")
			{
				// P_norm
				metric::P_norm<double> distance;
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::P_norm<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "Euclidean_thresholded")
			{
				// Euclidean_thresholded
				metric::Euclidean_thresholded<double> distance;
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::Euclidean_thresholded<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "Cosine")
			{
				// Cosine
				metric::Cosine<double> distance;
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::Cosine<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "Chebyshev")
			{
				// Chebyshev
				metric::Chebyshev<double> distance;
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::Chebyshev<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "Earth Mover Distance")
			{
				// Earth Mover Distance
				auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<double>(graph_w, graph_h);
				auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
				metric::EMD<double> distance(cost_mat, maxCost);
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::EMD<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "SSIM")
			{
				// SSIM
				metric::SSIM<double, Record> distance;
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::SSIM<double, Record>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			else if (metric_type == "TWED")
			{
				// TWED
				metric::TWED<double> distance(0, 1);
				relative_diff_mean = get_mean_distance_difference<Record, Graph, metric::TWED<double>>(graph, distance, dataset, randomized_indexes, isEstimate);
			}
			relative_diff_means.push_back(relative_diff_mean);

			if (verbose)
			{
				std::cout << metric_type << " relative_diff_mean: " << relative_diff_mean << std::endl;
			}
		}
		
		//Metrics metric;

		std::vector<double>::iterator min_result = std::min_element(relative_diff_means.begin(), relative_diff_means.end());
		auto best_index = std::distance(relative_diff_means.begin(), min_result);
		if (verbose)
		{
			std::cout << std::endl;
			std::cout << "Best metric: " << metric_type_names[best_index] << std::endl;
		}

		return metric_type_names[best_index];
	}
	
	template <typename Record, typename Graph, typename Metric = metric::Euclidean<double>>
	double get_mean_distance_difference(Graph &graph, Metric distance, std::vector<Record> dataset, std::vector<size_t> randomized_indexes, bool isEstimate)
	{		
		metric::SOM<Record, Graph, Metric> som(graph, Metric());
		if (isEstimate)
		{
			som.estimate(dataset, 50);
		}
		else
		{
			som.train(dataset);
		}
			
		auto iterations = 20;
		if (iterations > dataset.size())
		{
			iterations = dataset.size();
		}
		std::vector<double> relative_diffs;
		for (auto i = 0; i < iterations; i++)
		{
			for (auto j = 0; j < iterations; j++)
			{
				if (i != j)
				{
					auto dimR_1 = som.encode(dataset[randomized_indexes[i]]);
					auto bmu_1 = som.BMU(dataset[randomized_indexes[i]]);

					auto dimR_2 = som.encode(dataset[randomized_indexes[j]]);
					auto bmu_2 = som.BMU(dataset[randomized_indexes[j]]);
						
					// we get the same bmu for both records
					auto som_distance_1 = dimR_1[bmu_1] + dimR_2[bmu_1];
					auto som_distance_2 = dimR_1[bmu_2] + dimR_2[bmu_2];
					auto som_distance = min(som_distance_1, som_distance_2);
					auto direct_distance = distance(dataset[randomized_indexes[i]], dataset[randomized_indexes[j]]);
					auto diff = abs(abs(som_distance) - abs(direct_distance));
					auto relative_diff = diff / abs(direct_distance);
					relative_diffs.push_back(relative_diff);
					//std::cout << "som_distance: " << som_distance << " direct_distance: " << direct_distance << " diff: " << diff << " relative_diff: " << relative_diff << std::endl;
				}
			}
		}

		double relative_diff_sum = 0;
		for (auto item : relative_diffs)
		{
			relative_diff_sum += item;
		}
		double relative_diff_mean = relative_diff_sum / relative_diffs.size();

		return relative_diff_mean;
	}

}// end namespace metric
