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
	
	
template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(metric::SOM<Sample, Graph, Metric, Distribution> som_model) : som_model_(som_model)
{
	calculate_distance_matrix();
}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(std::vector<Sample>& samples, size_t nodesWidth, size_t nodesHeight) : 
	som_model_(Graph(nodesWidth, nodesHeight), Metric(), 0.8, 0.2, 20)
{
	som_model_.train(samples);

	calculate_distance_matrix();
}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(std::vector<Sample>& samples, Graph graph, Metric metric, 
	double start_learn_rate, double finish_learn_rate, size_t iterations, Distribution distribution) : 
	som_model_(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution)
{
	som_model_.train(samples);
	
	calculate_distance_matrix();
}


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
void kohonen_distance<D, Sample, Graph, Metric, Distribution>::calculate_distance_matrix()
{
	
	typedef boost::adjacency_list <boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property <boost::edge_weight_t, D>> Graph_t;
	typedef typename boost::graph_traits <Graph_t>::vertex_descriptor Vertex_descriptor;
	typedef std::pair<int, int> Edge;

	std::vector<Sample> nodes = som_model_.get_weights();

	std::vector<int> nodes_list(nodes.size());
	std::iota(nodes_list.begin(), nodes_list.end(), 0);
	
	std::vector<Edge> edge_vector;
	std::vector<D> weights;
    Metric distance;

	
	auto matrix = som_model_.get_graph().get_matrix();
	for (size_t i = 0; i < matrix.rows(); ++i) 
	{
		for (size_t j = i + 1; j < matrix.columns(); ++j) 
		{
			if (matrix(i, j) > 0)
			{
				edge_vector.push_back(Edge(i, j));
				weights.push_back(distance(nodes[i], nodes[j]));
			}
		}
	}

	Edge edge_array[edge_vector.size()];
	std::copy(edge_vector.begin(), edge_vector.end(), edge_array);
	int num_arcs = sizeof(edge_array) / sizeof(Edge);
	Graph_t g(edge_array, edge_array + num_arcs, weights.data(), nodes_list.size());

	typename boost::property_map<Graph_t, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, g);
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

