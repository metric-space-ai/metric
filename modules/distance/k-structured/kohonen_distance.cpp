/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 PANDA Team
*/
#ifndef _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_CPP
#define _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_CPP
#include "kohonen_distance.hpp"
#include "EMD.hpp"
#include <cmath>
#include <vector>

#endif

namespace metric {
	
	
template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(metric::SOM<Sample, Graph, Metric, Distribution> som_model) : som_model_(som_model)
{
	// calculate ground distance matrix between SOM nodes
	auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<typename Sample::value_type, Metric>(som_model_.get_weights());
	auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
	emd_distance_ = metric::EMD<D>(cost_mat, maxCost);
}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(std::vector<Sample>& samples, size_t nodesWidth, size_t nodesHeight) : 
	som_model_(Graph(nodesWidth, nodesHeight), Metric(), 0.8, 0.2, 20)
{
	som_model_.train(samples);
	
	// calculate ground distance matrix between SOM nodes
	auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<typename Sample::value_type, Metric>(som_model_.get_weights());
	auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
	emd_distance_ = metric::EMD<D>(cost_mat, maxCost);
}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(std::vector<Sample>& samples, Graph graph, Metric metric, 
	double start_learn_rate, double finish_learn_rate, size_t iterations, Distribution distribution) : 
	som_model_(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution)
{
	som_model_.train(samples);

	// calculate ground distance matrix between SOM nodes
	auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<typename Sample::value_type, Metric>(som_model_.get_weights());
	auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
	emd_distance_ = metric::EMD<D>(cost_mat, maxCost);
}


/*** distance measure on kohonen space. ***/
template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
auto kohonen_distance<D, Sample, Graph, Metric, Distribution>::operator()(const Sample& sample_1, const Sample& sample_2) -> distance_return_type
{
	// then we calculate distributions over SOM space for samples	
	auto reduced_1 = som_model_.encode(sample_1);
	auto reduced_2 = som_model_.encode(sample_2);

	// and finally calculate EDM distance for samples distributions over SOM space
	return emd_distance_(reduced_1, reduced_2);
}

}  // namespace metric

