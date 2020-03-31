/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 PANDA Team
*/

#ifndef _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_CPP
#define _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_CPP

#include "kohonen_distance.hpp"
#include "../../../3rdparty/blaze/Blaze.h"
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
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(const std::vector<Sample>& samples, size_t nodesWidth, size_t nodesHeight) :
	som_model_(Graph(nodesWidth, nodesHeight), Metric(), 0.8, 0.2, 20)
{
	som_model_.train(samples);

	calculate_distance_matrix();
}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
kohonen_distance<D, Sample, Graph, Metric, Distribution>::kohonen_distance(const std::vector<Sample>& samples, Graph graph, Metric metric,
	double start_learn_rate, double finish_learn_rate, size_t iterations, Distribution distribution) : 
	som_model_(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution)
{
	som_model_.train(samples);
	
	calculate_distance_matrix();
}


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
	std::vector<Sample> nodes = som_model_.get_weights();
    Metric distance;

	auto matrix = som_model_.get_graph().get_matrix();
	blaze::CompressedMatrix<D> blaze_matrix(matrix.rows(),  matrix.columns());
	for (size_t i = 0; i < matrix.rows(); ++i)
	{
		for (size_t j = i + 1; j < matrix.columns(); ++j) 
		{
			if (matrix(i, j) > 0)
			{
				blaze_matrix(i, j) = distance(nodes[i], nodes[j]);
				blaze_matrix(j, i) = distance(nodes[i], nodes[j]);
			}
		}
	}

	std::vector<D> distances;
	std::vector<int> predecessor;
	for (auto i = 0; i < nodes.size(); i++)
	{
		std:tie(distances, predecessor) = calculate_distance(blaze_matrix, i, matrix.rows());
		distance_matrix_.push_back(distances);
		predecessors_.push_back(predecessor);
	}
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
void kohonen_distance<D, Sample, Graph, Metric, Distribution>::print_shortest_path(int from_node, int to_node)
{
    if(to_node == from_node)
	{
		std::cout << to_node << " -> ";
    }
	else if(predecessors_[from_node][to_node] == -1)
	{
        std::cout << "No path from " << from_node << " to " << to_node << std::endl;
    }
	else
	{
        print_shortest_path(from_node,  predecessors_[from_node][to_node]);
        std::cout << to_node << " -> ";
    }
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
int kohonen_distance<D, Sample, Graph, Metric, Distribution>::get_closest_unmarked_node(const std::vector<D>& distance, const std::vector<bool>& mark, int nodes_count)
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
std::tuple<std::vector<D>, std::vector<int>> kohonen_distance<D, Sample, Graph, Metric, Distribution>::calculate_distance(const blaze::CompressedMatrix<D>& adjust_matrix, int from_node, int nodes_count)
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

}  // namespace metric

