/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#ifndef _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_HPP
#define _METRIC_DISTANCE_K_STRUCTURED_KOHONEN_HPP

#include "../../mapping/SOM.hpp"
#include "../../utils/graph.hpp"

namespace metric {

/**
 * @class Kohonen
 *
 * @brief 
 * 
 * The idea of the Kohonen distance is: to train a SOM on a dataset and then compute the EMD (shortest path between SOM's graph nodes) for two records in the Kohonen space.
 * 
 * The egdes of the SOM graph (distances between nodes) are the ground distance for the EMD.
 * Then for every record, to get a vector of distances between all SOM nodes, it is weights for ground distances.
 * Having the vectors and ground distances we can compute EMD. It is a Kohonen distance.
 *
 */
template <typename D, typename Sample, typename Graph = metric::Grid4, typename Metric = metric::Euclidean<D>, 
	typename Distribution = std::uniform_real_distribution<typename Sample::value_type>>
class Kohonen {
public:
    using distance_type = D;

    /**
     * @brief Construct a new Kohonen object. 
	 * 
	 * This construtor used when you have custom and pretrained SOM model. 
     *
     * @param som_model - trained SOM model
     * @param graph_optimization - type of optimization that can be performed on the graph. This optimizations delete some edges of the graph to fit better thetrain dataset. Can be: 
	 * <ul>
	 * <li>"none" - no optimization.</li> 
	 * <li>"sparsification" - graph will be sparced by deleting a percent of the largest edges. Percent of remained edges set by `graph_optimization_coef` parameter. </li> 
	 * <li>"reverse diffusion" - graph will be optimized using reverse diffusion method.</li>
	 * </ul>
     * @param graph_optimization_coef - coefficient using for optimization. See `graph_optimization` description for meanings with concrete optimization type.
     */

    Kohonen(metric::SOM<Sample, Graph, Metric, Distribution>&& som_model, std::string graph_optimization = "none", double graph_optimization_coef = 1.0);

    /**
     * @brief Construct a new Kohonen object
	 * 
	 * This construtor used when you have custom and pretrained SOM model. 
     *
     * @param som_model - trained SOM model
     * @param graph_optimization - type of optimization that can be performed on the graph. This optimizations delete some edges of the graph to fit better thetrain dataset. Can be: 
	 * <ul>
	 * <li>"none" - no optimization.</li> 
	 * <li>"sparsification" - graph will be sparced by deleting a percent of the largest edges. Percent of remained edges set by `graph_optimization_coef` parameter. </li> 
	 * <li>"reverse diffusion" - graph will be optimized using reverse diffusion method.</li>
	 * </ul>
     * @param graph_optimization_coef - coefficient using for optimization. See `graph_optimization` description for meanings with concrete optimization type.
     */
	Kohonen(const metric::SOM<Sample, Graph, Metric, Distribution>& som_model, std::string graph_optimization = "none", double graph_optimization_coef = 1.0);

    /**
     * @brief Construct a new Kohonen object
	 * 
	 * This construtor is for quick distance object creation. Just set width and height of the graph (use Grid4, Grid6 or Grid8) and constructor will create Graph, SOM model, train SOM and object will be ready to use. 
     *
     * @param samples - samples for SOM train.
     * @param nodesWidth - width of the SOM grid.
     * @param nodesHeight - height of the SOM grid.
     * @param graph_optimization - type of optimization that can be performed on the graph. This optimizations delete some edges of the graph to fit better thetrain dataset. Can be: 
	 * <ul>
	 * <li>"none" - no optimization.</li> 
	 * <li>"sparsification" - graph will be sparced by deleting a percent of the largest edges. Percent of remained edges set by `graph_optimization_coef` parameter. </li> 
	 * <li>"reverse diffusion" - graph will be optimized using reverse diffusion method.</li>
	 * </ul>
     * @param graph_optimization_coef - coefficient using for optimization. See `graph_optimization` description for meanings with concrete optimization type.
     */
	Kohonen(const std::vector<Sample>& samples, size_t nodesWidth, size_t nodesHeight, std::string graph_optimization = "none", double graph_optimization_coef = 1.0);

    /**
     * @brief Construct a new Kohonen object
	 * 
	 * This construtor is for detailed creation of the distance object. SOM model will be created and trained by constructor and object will be ready to use.
     *
     * @param samples - samples for SOM train.
     * @param graph - graph for SOM model.
     * @param metric - metric for SOM model
     * @param start_learn_rate - learning rate for SOM model at the start of train.
     * @param finish_learn_rate - learning rate for SOM model at the end of train.
     * @param iterations - number of iterations for SOM model train.
     * @param distribution - initial distribution of the weights of the SOM model before train.
     * @param graph_optimization - type of optimization that can be performed on the graph. This optimizations delete some edges of the graph to fit better thetrain dataset. Can be: 
	 * <ul>
	 * <li>"none" - no optimization.</li> 
	 * <li>"sparsification" - graph will be sparced by deleting a percent of the largest edges. Percent of remained edges set by `graph_optimization_coef` parameter. </li> 
	 * <li>"reverse diffusion" - graph will be optimized using reverse diffusion method.</li>
	 * </ul>
     * @param graph_optimization_coef - coefficient using for optimization. See `graph_optimization` description for meanings with concrete optimization type.
     */
	Kohonen(
	    const std::vector<Sample>& samples,
	    Graph graph,
	    Metric metric = Metric(),
	    double start_learn_rate = 0.8,
	    double finish_learn_rate = 0.0,
	    size_t iterations = 20,
		Distribution distribution = Distribution(-1, 1), 
		std::string graph_optimization = "none", 
		double graph_optimization_coef = 1.0
    );

    /**
     * @brief Compute the EMD for two records in the Kohonen space.
     *
     * @param sample_1 first sample
     * @param sample_2 second sample
     * @return distance on kohonen space
     */
    distance_type operator()(const Sample& sample_1, const Sample& sample_2) const;

    /**
     * @brief
	 * Recursive function that reconstructs the shortest path backwards node by node and print it
     *
     * @param from_node - index of the SOM's graph start node
     * @param to_node second sample - index of the SOM's graph end node
     */
	void print_shortest_path(int from_node, int to_node) const;

    /**
     * @brief
	 * Recursive function that reconstructs the shortest path backwards node by node
     *
     * @param from_node - index of the SOM's graph start node
     * @param to_node second sample - index of the SOM's graph end node
     */
	std::vector<int> get_shortest_path(int from_node, int to_node) const;

    /**
     * @brief
	 * Distortion estimate method. it is a measure of the nonlinearity of a dataset. 
	 * Calculates by taking the factor between all euclidean and Kohonen distances of a dataset. 
	 * Then rescaling these factors from all pair-wise comparisons (like rescaling a histogram). 
	 * And the distortion is the variance of this histogram. 
     *
     * @param samples - dataset for estimation. 
     */
	double distortion_estimate(const std::vector<Sample>& samples);
	
    /**
     * @brief
	 * SOM model used in the core of Kohonen distance object.
     */
	metric::SOM<Sample, Graph, Metric, Distribution> som_model;
	
    /**
     * @brief
	 * Matrix with EMD distances between SOM nodes.
     */
	std::vector<std::vector<D>> distance_matrix;

    /**
     * @brief
	 * Predecessors matrix for all SOM nodes.
     */
	std::vector<std::vector<int>> predecessors;

private:
	
    /**
     * @brief
	 * Type of optimization that can be performed on the graph. This optimizations delete some edges of the graph to fit better thetrain dataset. Can be: 
	 * <ul>
	 * <li>"none" - no optimization.</li> 
	 * <li>"sparsification" - graph will be sparced by deleting a percent of the largest edges. Percent of remained edges set by `graph_optimization_coef` parameter. </li> 
	 * <li>"reverse diffusion" - graph will be optimized using reverse diffusion method.</li>
	 * </ul>
     */
	std::string graph_optimization_ = "none";

    /**
     * @brief
	 * Coefficient using for optimization. See `graph_optimization` description for meanings with concrete optimization type.
     */
	double graph_optimization_coef_ = 1;
	
    /**
     * @brief
	 * Method calculates matrix with direct (based for SOM) distances between SOM nodes.
     */
	void calculate_distance_matrix();
	
	
    /**
     * @brief
	 * Method optimizing graph based for the SOM according to `graph_optimization` and `graph_optimization_coef`.
     *
     * @param direct_distance_matrix - direct (based for SOM) distance matrix between SOM nodes. This matrix will be modifed while optimization.
     */
	void optimize_graph(blaze::CompressedMatrix<D>& direct_distance_matrix);
	
    /**
     * @brief
	 * Recursive function that reconstructs the shortest path backwards node by node
     *
     * @param from_node - index of the SOM's graph start node
     * @param to_node second sample - index of the SOM's graph end node
     */
	std::vector<int> get_shortest_path_(std::vector<int> &path, int from_node, int to_node) const;

	/**
	 * @brief return closest node from SOM's graph that was not visited while paths calculations.
	 * @return the index of closest nodes was not visited
	 */
	int get_closest_unmarked_node(
	    const std::vector<D>& distance,
	    const std::vector<bool>& mark,
	    int numOfVertices
    ) const;

	/**
	 * @brief
	 * Computes the shortest path
	 */
	auto calculate_distance(const blaze::CompressedMatrix<D>& adjMatrix, int from_node, int num) const
	    -> std::tuple<std::vector<D>, std::vector<int>>;

	std::vector<std::pair<size_t, size_t>> sort_indexes(const blaze::CompressedMatrix<D>& matrix);
};

}  // namespace metric

#include "Kohonen.cpp"

#endif  // Header Guard
