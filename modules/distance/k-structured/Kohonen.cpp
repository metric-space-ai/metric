/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 PANDA Team
*/

#include "Kohonen.hpp"
#include "../../../3rdparty/blaze/Blaze.h"
#include <cmath>
#include <vector>

namespace metric {

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(metric::SOM<Sample, Graph, Metric, Distribution>&& som_model)
    : som_model(som_model)
{
	calculate_distance_matrix();
}

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(const metric::SOM<Sample, Graph, Metric, Distribution>& som_model)
    : som_model(som_model)
{
	calculate_distance_matrix();
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(const std::vector<Sample>& samples, size_t nodesWidth, size_t nodesHeight)
    : som_model(Graph(nodesWidth, nodesHeight), Metric(), 0.8, 0.2, 20)
{
	som_model.train(samples);

	calculate_distance_matrix();
}
	

template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
Kohonen<D, Sample, Graph, Metric, Distribution>::Kohonen(
    const std::vector<Sample>& samples,
    Graph graph,
    Metric metric,
	double start_learn_rate,
	double finish_learn_rate,
	size_t iterations,
	Distribution distribution
)
	 : som_model(graph, metric, start_learn_rate, finish_learn_rate, iterations, distribution)
{
	som_model.train(samples);
	
	calculate_distance_matrix();
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
auto Kohonen<D, Sample, Graph, Metric, Distribution>::operator()(const Sample& sample_1, const Sample& sample_2) const
    -> distance_type
{
	// then we calculate distributions over SOM space for samples	
	auto bmu_1 = som_model.BMU(sample_1);
	auto bmu_2 = som_model.BMU(sample_2);

	return distance_matrix[bmu_1][bmu_2];
}


template <typename D, typename Sample, typename Graph, typename Metric, typename Distribution>
void Kohonen<D, Sample, Graph, Metric, Distribution>::calculate_distance_matrix()
{
	std::vector<Sample> nodes = som_model.get_weights();
    Metric distance;

	auto matrix = som_model.get_graph().get_matrix();
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
		std::tie(distances, predecessor) = calculate_distance(blaze_matrix, i, matrix.rows());
		distance_matrix.push_back(distances);
		predecessors.push_back(predecessor);
	}
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

}  // namespace metric

