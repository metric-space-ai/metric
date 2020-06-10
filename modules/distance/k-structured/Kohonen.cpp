/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 PANDA Team
*/

#include "Kohonen.hpp"
#include "../../../3rdparty/blaze/Blaze.h"
#include "../../../modules/utils/poor_mans_quantum.hpp"
#include <cmath>
#include <vector>

namespace metric {

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(
	metric::SOM<Sample, Graph, Metric, Distribution>&& som_model,
	const std::vector<Sample>& samples,
	bool use_sparsification, double sparsification_coef, 
	bool use_reverse_diffusion, size_t reverse_diffusion_neighbors
	)
    : som_model(som_model), 
	use_sparsification_(use_sparsification), sparsification_coef_(sparsification_coef), 
	use_reverse_diffusion_(use_reverse_diffusion), reverse_diffusion_neighbors_(reverse_diffusion_neighbors)
{
	this->metric = som_model.get_metric();
	calculate_distance_matrix(samples);
}

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(
	const metric::SOM<Sample, Graph, Metric, Distribution>& som_model, 
	const std::vector<Sample>& samples,
	bool use_sparsification, double sparsification_coef, 
	bool use_reverse_diffusion, size_t reverse_diffusion_neighbors
	)
    : som_model(som_model), 
	use_sparsification_(use_sparsification), sparsification_coef_(sparsification_coef), 
	use_reverse_diffusion_(use_reverse_diffusion), reverse_diffusion_neighbors_(reverse_diffusion_neighbors)
{
	this->metric = som_model.get_metric();
	calculate_distance_matrix(samples);
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(
	const std::vector<Sample>& samples, size_t nodesWidth, size_t nodesHeight, 
	bool use_sparsification, double sparsification_coef, 
	bool use_reverse_diffusion, size_t reverse_diffusion_neighbors
	)
    : som_model(Graph(nodesWidth, nodesHeight), Metric(), 0.8, 0.2, 20), 
	use_sparsification_(use_sparsification), sparsification_coef_(sparsification_coef), 
	use_reverse_diffusion_(use_reverse_diffusion), reverse_diffusion_neighbors_(reverse_diffusion_neighbors)
{
	this->metric = som_model.get_metric();
	som_model.train(samples);

	calculate_distance_matrix(samples);
}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(
    const std::vector<Sample>& samples,
    Graph graph,
    Metric metric,
	double start_learn_rate,
	double finish_learn_rate,
	size_t iterations,
	Distribution distribution, 
	bool use_sparsification, 
	double sparsification_coef, 
	bool use_reverse_diffusion, 
	size_t reverse_diffusion_neighbors
	)
	 : som_model(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution), 
	use_sparsification_(use_sparsification), sparsification_coef_(sparsification_coef), 
	use_reverse_diffusion_(use_reverse_diffusion), reverse_diffusion_neighbors_(reverse_diffusion_neighbors)
{
	this->metric = som_model.get_metric();
	som_model.train(samples);
	
	calculate_distance_matrix(samples);
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
auto Kohonen<D, Sample, Graph, Metric, Distribution>::operator()(const Sample& sample_1, const Sample& sample_2) const
    -> distance_type
{
	// then we calculate distributions over SOM space for samples	
	auto bmu_1 = som_model.BMU(sample_1);
	auto bmu_2 = som_model.BMU(sample_2);

	std::vector<Sample> nodes = som_model.get_weights();
	
	auto direct_distance = metric(sample_1, sample_2);
	
	double to_nearest_1 = metric(sample_1, nodes[bmu_1]);
	double to_nearest_2 = metric(nodes[bmu_2], sample_2);

	if (direct_distance < to_nearest_1 + to_nearest_2)
	{
		return direct_distance;
	}

	return to_nearest_1 + distance_matrix[bmu_1][bmu_2] + to_nearest_2;
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
void Kohonen<D, Sample, Graph, Metric, Distribution>::calculate_distance_matrix(const std::vector<Sample>& samples)
{
	std::vector<Sample> nodes = som_model.get_weights();

	if (use_reverse_diffusion_)
	{
		make_reverese_diffusion(samples);
	}

	auto matrix = som_model.get_graph().get_matrix();
	blaze::CompressedMatrix<D> blaze_matrix(matrix.rows(),  matrix.columns());
	for (size_t i = 0; i < matrix.rows(); ++i)
	{
		for (size_t j = i + 1; j < matrix.columns(); ++j) 
		{
			if (matrix(i, j) > 0)
			{
				blaze_matrix(i, j) = metric(nodes[i], nodes[j]);
				blaze_matrix(j, i) = metric(nodes[i], nodes[j]);
			}
		}
	}

	if (use_sparsification_)
	{
		sparcify_graph(blaze_matrix);
	}

	matrix = som_model.get_graph().get_matrix();

	std::vector<D> distances;
	std::vector<int> predecessor;
	for (auto i = 0; i < nodes.size(); i++)
	{
		std::tie(distances, predecessor) = calculate_distance(blaze_matrix, i, matrix.rows());
		distance_matrix.push_back(distances);
		predecessors.push_back(predecessor);
	}
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
void Kohonen<D, Sample, Graph, Metric, Distribution>::sparcify_graph(blaze::CompressedMatrix<D>& direct_distance_matrix)
{
	auto matrix = som_model.get_graph().get_matrix();
	auto sorted_pairs = sort_indexes(direct_distance_matrix); 
	for (size_t i = 0; i < sorted_pairs.size() * (1 - sparsification_coef_); ++i)
	{
		auto p = sorted_pairs[i];
		matrix(p.first, p.second) = 0;
		direct_distance_matrix(p.first, p.second) = 0;
		direct_distance_matrix(p.second, p.first) = 0;
	}

	som_model.get_graph().updateEdges(matrix);
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
void Kohonen<D, Sample, Graph, Metric, Distribution>::make_reverese_diffusion(const std::vector<Sample>& samples)
{
	metric::Redif redif(samples, reverse_diffusion_neighbors_, 10, metric);
	
	auto [redif_encoded, indicies] = redif.encode(som_model.get_weights());

	som_model.updateWeights(redif_encoded);
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
void Kohonen<D, Sample, Graph, Metric, Distribution>::print_shortest_path(int from_node, int to_node) const
{
    if(to_node == from_node)
	{
		std::cout << to_node << " -> ";
    }
	else if(predecessors[from_node][to_node] == -1)
	{
        std::cout << "No path from " << from_node << " to " << to_node << std::endl;
    }
	else
	{
        print_shortest_path(from_node,  predecessors[from_node][to_node]);
        std::cout << to_node << " -> ";
    }
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
std::vector<int> Kohonen<D, Sample, Graph, Metric, Distribution>::get_shortest_path(int from_node, int to_node) const
{
	std::vector<int> path;
	return get_shortest_path_(path, from_node, to_node);
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
double Kohonen<D, Sample, Graph, Metric, Distribution>::distortion_estimate(const std::vector<Sample>& samples)
{
	metric::Euclidean<D> euclidean_distance;
	std::vector<D> factor_histogram;
	for (size_t i = 0; i < samples.size(); ++i)
	{
		for (size_t j = 0; j < samples.size(); ++j)
		{
			if (i != j)
			{
				auto euclidean = euclidean_distance(samples[i], samples[j]);
				auto kohonen = operator()(samples[i], samples[j]);
				if (euclidean != 0 && kohonen != 0)
				{
					factor_histogram.push_back(kohonen / euclidean);
				}
				else 
				{
					factor_histogram.push_back(0);
				}
			}
		}
	}
	
    metric::PMQ<Discrete<D>, double> pmq(factor_histogram);

	return pmq.variance();
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
std::vector<int> Kohonen<D, Sample, Graph, Metric, Distribution>::get_shortest_path_(std::vector<int> &path, int from_node, int to_node) const
{
    if(to_node == from_node)
	{
		path.push_back(to_node);
    }
	else if(predecessors[from_node][to_node] == -1)
	{
    }
	else
	{
        get_shortest_path_(path, from_node,  predecessors[from_node][to_node]);
		path.push_back(to_node);
    }

	return path;
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
int Kohonen<D, Sample, Graph, Metric, Distribution>::get_closest_unmarked_node(
    const std::vector<D>& distance,
    const std::vector<bool>& mark,
    int nodes_count
) const
{
    D minDistance = INFINITY;
    int closestUnmarkedNode = -1;
    for(int i = 0; i < nodes_count; i++)
	{
        if( (!mark[i]) && (minDistance >= distance[i]) )
		{
            minDistance = distance[i];
            closestUnmarkedNode = i;
        }
    }
    return closestUnmarkedNode;
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
auto Kohonen<D, Sample, Graph, Metric, Distribution>::calculate_distance(
    const blaze::CompressedMatrix<D>& adjust_matrix,
    int from_node,
    int nodes_count
) const
    -> std::tuple<std::vector<D>, std::vector<int>>
{
    std::vector<bool> mark(nodes_count);
    std::vector<D> distances(nodes_count);
    std::vector<int> predecessor(nodes_count);

	// initialize

    for(int i = 0; i < nodes_count; i++)
	{
        mark[i] = false;
        predecessor[i] = -1;
        distances[i] = INFINITY;
    }
    distances[from_node] = 0;

	//

    int closestUnmarkedNode;
    int count = 0;
    while(count < nodes_count)
	{
        closestUnmarkedNode = get_closest_unmarked_node(distances, mark, nodes_count);
		mark[closestUnmarkedNode] = true;
		for (int i = 0; i < nodes_count; i++)
		{
			if (!mark[i] && adjust_matrix(closestUnmarkedNode, i) > 0)
			{
				if (distances[i] > distances[closestUnmarkedNode] + adjust_matrix(closestUnmarkedNode, i))
				{
					distances[i] = distances[closestUnmarkedNode] + adjust_matrix(closestUnmarkedNode, i);
					predecessor[i] = closestUnmarkedNode;
				}
			}
		}
        count++;
    }

	return { distances, predecessor };
}

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
std::vector<std::pair<size_t, size_t>> Kohonen<D, Sample, Graph, Metric, Distribution>::sort_indexes(const blaze::CompressedMatrix<D>& matrix) 
{

	// initialize original index locations
	// std::vector<std::pair<size_t, size_t>>& idx

	std::vector<D> v;
	std::vector<std::pair<size_t, size_t>> idx_pairs;
	for (size_t i = 0; i < matrix.rows(); ++i)
	{
		for (size_t j = i + 1; j < matrix.columns(); ++j)
		{
			if (matrix(i, j) > 0)
			{
				v.push_back(matrix(i, j));
				idx_pairs.push_back({i, j});
			}
		}
	}
	std::vector<size_t> idx(v.size());
	std::iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	// using std::stable_sort instead of std::sort
	// to avoid unnecessary index re-orderings
	// when v contains elements of equal values 
	stable_sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

	
	std::vector<std::pair<size_t, size_t>> sorted_idx_pairs;
	for (size_t i = 0; i < idx.size(); ++i)
	{
		sorted_idx_pairs.push_back(idx_pairs[idx[i]]);
	}

	return sorted_idx_pairs;
}

}  // namespace metric

