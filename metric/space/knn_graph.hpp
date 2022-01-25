/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2020 Panda Team

*/

#ifndef _METRIC_SPACE_KNN_GRAPH_HPP
#define _METRIC_SPACE_KNN_GRAPH_HPP

#include "modules/utils/graph.hpp"
#include "modules/utils/type_traits.hpp"

#include <type_traits>
#include <vector>

namespace metric {

/**
 * @class KNNGraph
 * @brief Fast hierarchical method algorithm that constructs an approximate kNN graph.
 * The method is simple and it works with any type of data for which a distance function can be provided.
 * Algorithm has two parts. In the first part we create a crude approximation of the graph by subdividing
 * the dataset until each subset reaches given max_bruteforce_size.
 * In the second part this approximation is iteratively fine-tuned by combining the first algorithm with NN-descent
 * method.
 *
 */
template <typename Sample, typename Distance, typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
class KNNGraph : public Graph<WeightType, isDense, isSymmetric> {
  public:
	using distance_type = typename Distance::distance_type;
	/**
	 * @brief Construct a new KNN Graph object
	 *
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same_v<Sample, type_traits::index_value_type_t<Container>>>>
	KNNGraph(const Container &X, size_t neighbors_num, size_t max_bruteforce_size, int max_iterations = 100,
			 double update_range = 0.02);

	///**
	// * @brief Construct a new KNN Graph object
	// *
	// */
	// KNNGraph(Tree<Sample, Distance>& tree, size_t neighbors_num, size_t max_bruteforce_size, int max_iterations =
	// 100, double update_range = 0.02);

	///**
	// * @brief Construct a new KNN Graph object
	// *
	// */
	// KNNGraph(std::vector<std::vector<typename Distance::value_type>> distance_matrix);

	size_t neighbors_num() const { return _neighbors_num; }

	const Sample &get_node_data(size_t i) const { return _nodes[i]; }

	std::vector<std::size_t> gnnn_search(const Sample &query, int max_closest_num, int iterations = 10,
										 int num_greedy_moves = -1, int num_expansions = -1);

	/**
	 * @brief return value with index equals idx
	 * @param idx index of searching value
	 */
	const Sample &operator[](std::size_t idx) const { return _nodes[idx]; }

	/**
	 * @brief return distance between elements with indexes i and j
	 * @param i index of the first element
	 * @param j index of the second element
	 */
	distance_type operator()(std::size_t i, std::size_t j) const { return _distance_matrix[i][j]; }

	/**
	 * @brief insert new element into the graph
	 * @param p new element
	 * return index of the inserted element
	 */
	std::size_t insert(const Sample &p);

	/**
	 * @brief insert set of the new elements into the graph
	 * @param p - container with new elements
	 * @return vector of indexes of the new elements
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same<
				  Sample, typename std::decay<decltype(std::declval<Container &>().operator[](0))>::type>::value>>
	std::vector<std::size_t> insert(const Container &p);

	/**
	 * @brief insert new element into the graph if distance between this element and its NN is greater than threshold
	 * @param p new element
	 * @param threshold distance threshold
	 * @return pair consist of index and result of insertion
	 */
	std::pair<std::size_t, bool> insert_if(const Sample &p, distance_type threshold);

	/**
	 * @brief append data records into the Matrix only if distance bigger than a threshold
	 *
	 * @param items set of new records
	 * @param treshold distance threshold
	 * @return vector of pairs consists of indexes and results of insertion
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same<
				  Sample, typename std::decay<decltype(std::declval<Container &>().operator[](0))>::type>::value>>
	std::vector<std::pair<std::size_t, bool>> insert_if(const Container &items, distance_type threshold);
	/**
	 * @brief find nearest neighbour
	 * @param p searching value
	 * @return index of NN in graph
	 */
	std::size_t nn(const Sample &p);

	/**
	 * @brief find K nearest neighbours
	 * @param p searching value
	 * @param K amount of neighbours
	 * @return vector of indexes of NN's in graph
	 */
	std::vector<std::size_t> knn(const Sample &p, std::size_t K) const;

	/**
	 * @brief find all nearest neighbours in sphere of radius threshold
	 * @param p searching value
	 * @param threshold  radius of threshold sphere
	 * @return vector of indexes of NN's in graph and distances to searching value
	 */
	auto rnn(const Sample &x, distance_type threshold) const -> std::vector<std::pair<std::size_t, distance_type>>;

	/**
	 * @brief erase element from graph
	 * @param idx index of erasing element
	 */
	void erase(std::size_t idx);

	/**
	 * @brief return size of graph
	 *
	 */
	std::size_t size() const { return _nodes.size(); }

  protected:
	size_t _neighbors_num = 1;
	size_t _max_bruteforce_size = 10;

	int _max_iterations = 100;
	double _update_range = 0.02;

	bool _not_more_neighbors = false;

	std::vector<Sample> _nodes;
	std::vector<std::vector<distance_type>> _distance_matrix;

  private:
	/**
	 * @brief
	 *
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same_v<Sample, type_traits::index_value_type_t<Container>>>>
	void construct(const Container &X);

	/**
	 * @brief
	 *
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same_v<Sample, type_traits::index_value_type_t<Container>>>>
	void calculate_distance_matrix(const Container &X);

	/**
	 * @brief
	 *
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same_v<Sample, type_traits::index_value_type_t<Container>>>>
	void make_edge_pairs(const Container &X);

	/**
	 * @brief
	 *
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same_v<Sample, type_traits::index_value_type_t<Container>>>>
	std::vector<std::pair<size_t, size_t>> brute_force(const Container &samples, const std::vector<int> &ids);

	/**
	 * @brief
	 *
	 */
	template <typename Container,
			  typename = std::enable_if<std::is_same_v<Sample, type_traits::index_value_type_t<Container>>>>
	std::vector<std::pair<size_t, size_t>> random_pair_division(const Container &samples, const std::vector<int> &ids,
																int max_size);

	/**
	 * @brief
	 *
	 */
	template <typename T1> std::vector<size_t> sort_indexes(const std::vector<T1> &v);
};

} // namespace metric
#include "knn_graph.cpp"
#endif
