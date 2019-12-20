/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 PANDA Team
*/
#ifndef _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_CPP
#define _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_CPP
#include "kohonen_distance.hpp"
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <cmath>
#include <vector>

#endif

namespace metric {
	
	
//template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
//kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(metric::SOM<Sample, Graph, Metric, Distribution> som_model) : som_model_(som_model)
//{
//	// calculate ground distance matrix between SOM nodes
//	//auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<typename Sample::value_type, Metric>(som_model_.get_weights());
//	//auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
//	//emd_distance_ = metric::EMD<D>(cost_mat, maxCost);
//
//	calculate_distance_matrix(som_model_.get_weights(), nodesWidth, nodesHeight);
//}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(std::vector<Sample>& samples, size_t nodesWidth, size_t nodesHeight) : 
	som_model_(Graph(nodesWidth, nodesHeight), Metric(), 0.8, 0.2, 20)
{
	som_model_.train(samples);
	
	// calculate ground distance matrix between SOM nodes
	//auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<typename Sample::value_type, Metric>(som_model_.get_weights());
	//auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
	//emd_distance_ = metric::EMD<D>(cost_mat, maxCost);

	calculate_distance_matrix(som_model_.get_weights(), nodesWidth, nodesHeight);
}
	

//template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
//kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(std::vector<Sample>& samples, Graph graph, Metric metric, 
//	double start_learn_rate, double finish_learn_rate, size_t iterations, Distribution distribution) : 
//	som_model_(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution)
//{
//	som_model_.train(samples);
//
//	// calculate ground distance matrix between SOM nodes
//	//auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<typename Sample::value_type, Metric>(som_model_.get_weights());
//	//auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
//	//emd_distance_ = metric::EMD<D>(cost_mat, maxCost);
//
//	size_t nodesWidth = ;
//	size_t nodesHeight = ;
//
//	calculate_distance_matrix(samples, nodesWidth, nodesHeight);
//}


/*** distance measure on kohonen space. ***/
template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
auto kohonen_distance<D, Sample, Graph, Metric, Distribution>::operator()(const Sample& sample_1, const Sample& sample_2) -> distance_return_type
{
	// then we calculate distributions over SOM space for samples	
	auto bmu_1 = som_model_.BMU(sample_1);
	auto bmu_2 = som_model_.BMU(sample_2);

	return distance_matrix_[bmu_1][bmu_2];
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
void kohonen_distance<D, Sample, Graph, Metric, Distribution>::calculate_distance_matrix(std::vector<Sample> samples, size_t nodesWidth, size_t nodesHeight)
{
	
	typedef boost::adjacency_list <boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property <boost::edge_weight_t, D>> Graph_t;
	typedef boost::graph_traits <Graph_t>::vertex_descriptor Vertex_descriptor;
	typedef std::pair<int, int> Edge;

	std::vector<int> nodes_list(samples.size());
	std::iota(nodes_list.begin(), nodes_list.end(), 0);
	
	std::vector<Edge> edge_vector;
	std::vector<D> weights;
    Metric distance;
	// horisontal edges
	for (size_t c1 = 0; c1 < nodesWidth; ++c1) 
	{
        for (size_t c2 = 0; c2 < nodesHeight - 1; ++c2) 
		{
			int node = c1 + c2 * nodesWidth;
			edge_vector.push_back(Edge(node, node + nodesWidth));
			weights.push_back(distance(samples[node], samples[node + nodesWidth]));
        }
    }
	// vertical edges
	for (size_t c1 = 0; c1 < nodesHeight; ++c1) 
	{
        for (size_t c2 = 0; c2 < nodesWidth - 1; ++c2) 
		{
			int node = c1 * nodesWidth + c2;
			edge_vector.push_back(Edge(node, node + 1));
			weights.push_back(distance(samples[node], samples[node + 1]));
        }
    }

	Edge edge_array[edge_vector.size()];
	std::copy(edge_vector.begin(), edge_vector.end(), edge_array);
	int num_arcs = sizeof(edge_array) / sizeof(Edge);
	Graph_t g(edge_array, edge_array + num_arcs, weights.data(), nodes_list.size());

	boost::property_map<Graph_t, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, g);
	std::vector<Vertex_descriptor> p(num_vertices(g));
	std::vector<D> d(num_vertices(g));

	for (auto i = 0; i < nodes_list.size(); i++)
	{
		Vertex_descriptor s = vertex(nodes_list[i], g);

		dijkstra_shortest_paths(g, s,
								predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, g))).
								distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, g))));

		distance_matrix_.push_back(d);
	}
}

}  // namespace metric

