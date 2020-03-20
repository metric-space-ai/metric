/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_UTILS_GRAPH_HPP
#define _METRIC_UTILS_GRAPH_HPP

#include "../../3rdparty/blaze/Blaze.h"
#include "../space/tree.hpp"

#include <stack>
#include <type_traits>


namespace metric {



// Graph based on blaze-lib

/**
 * @class Graph
 * @brief
 * 
 */
template <typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
class Graph {
        static constexpr bool isWeighted = !std::is_same<WeightType, bool>::value; // used only in getNeighboursOld, TODO remive if method removed

    typedef typename std::conditional<
        isDense,
        blaze::DynamicMatrix<WeightType>,
        blaze::CompressedMatrix<WeightType>
        >::type InnerMatrixType;

    typedef typename std::conditional<
        isSymmetric,
        blaze::SymmetricMatrix<InnerMatrixType>,
        InnerMatrixType
        >::type MatrixType;

public:
    /**
     * @brief Construct a new Graph object
     * 
     * @param nodesNumber 
     */
    explicit Graph(size_t nodesNumber);

    /**
     * @brief Construct a new Graph object
     * 
     */
    Graph();

    /**
     * @brief Construct a new Graph object
     * 
     * @param edgesPairs 
     */
    Graph(const std::vector<std::pair<size_t, size_t>> & edgesPairs);

    /**
     * @brief Construct a new Graph object
     * 
     * @param matrix 
     */
    Graph(MatrixType && matrix);

    /**
     * @brief Destroy the Graph object
     * 
     */
    ~Graph();

    /**
     * @brief 
     * 
     * @return
     */
    size_t getNodesNumber();

    /**
     * @brief 
     * 
     * @return
     */
    bool isValid();

    /**
     * @brief 
     * 
     * @param nodeIndex 
     * @param maxDeep 
     * @return
     */
    std::vector<std::vector<size_t>> getNeighboursOld(const size_t nodeIndex, const size_t maxDeep);

    /**
     * @brief 
     * 
     * @param nodeIndex 
     * @param maxDeep 
     * @return
     */
    template <typename T = WeightType, bool denseFlag = isDense>
    typename std::enable_if_t<!std::is_same<T, bool>::value, std::vector<std::vector<size_t>>>
    getNeighbours(const size_t nodeIndex, const size_t maxDeep); // not bool

    /**
     * @brief 
     * 
     * @param nodeIndex 
     * @param maxDeep 
     * @return
     */
    template <typename T = WeightType, bool denseFlag = isDense>
    typename std::enable_if_t<std::is_same<T, bool>::value && !denseFlag, std::vector<std::vector<size_t>>>
    getNeighbours(const size_t nodeIndex, const size_t maxDeep); // bool, not dense

    /**
     * @brief
     * 
     * @param nodeIndex 
     * @param maxDeep 
     * @return
     */
    template <typename T = WeightType, bool denseFlag = isDense>
    typename std::enable_if_t<std::is_same<T, bool>::value && denseFlag, std::vector<std::vector<size_t>>>
    getNeighbours(const size_t nodeIndex, const size_t maxDeep); // bool, dense

    /**
     * @brief 
     * 
     * @return
     */
    MatrixType get_matrix();

    /**
     * @brief 
     * 
     * @param edgesPairs 
     */
    void buildEdges(const std::vector<std::pair<size_t, size_t>> &edgesPairs);

protected:
    size_t nodesNumber;
    bool valid;

    MatrixType m;

    size_t modularPow(const size_t base, const size_t exponent, const size_t modulus);
};




/**
 * @class Grid4
 * @brief
 * 
 */
class Grid4 : public Graph<> {
public:
    /**
     * @brief Construct a new Grid 4 object
     * 
     * @param nodesNumber 
     */
    explicit Grid4(size_t nodesNumber);

    /**
     * @brief Construct a new Grid 4 object
     * 
     * @param width 
     * @param height 
     */
    Grid4(size_t width, size_t height);
private:
    void construct(size_t width, size_t height);
};



/**
 * @class Grid6
 * @brief
 * 
 */
class Grid6 : public Graph<> {
public:
    /**
     * @brief Construct a new Grid 6 object
     * 
     * @param nodesNumber 
     */
    explicit Grid6(size_t nodesNumber);
    
    /**
     * @brief Construct a new Grid 6 object
     * 
     * @param width 
     * @param height 
     */
    Grid6(size_t width, size_t height);
private:
    void construct(size_t width, size_t height);
};



/**
 * @class Grid8
 * @brief
 * 
 */

class Grid8 : public Graph<> {
public:
    /**
     * @brief Construct a new Grid 8 object
     * 
     * @param nodesNumber 
     */
    explicit Grid8(size_t nodesNumber);
    /**
     * @brief Construct a new Grid 8 object
     * 
     * @param width 
     * @param height 
     */
    Grid8(size_t width, size_t height);
private:
    void construct(size_t width, size_t height);
};



/**
 * @class Paley
 * @brief
 * 
 */
class Paley : public Graph<> {
public:
    /**
     * @brief Construct a new Paley object
     * 
     * @param nodesNumber 
     */
    explicit Paley(size_t nodesNumber);
};



/**
 * @class LPS
 * @brief
 * 
 */
class LPS : public Graph<> {
public:
    /**
     * @brief Construct a new LPS object
     * 
     * @param nodesNumber 
     */
    explicit LPS(size_t nodesNumber);
private:
    bool miller_rabin_pass(const size_t a, const size_t s,
                           const size_t d, const size_t nodesNumber);

    bool millerRabin(const size_t nodesNumber);
};




/**
 * @class Margulis
 * @brief
 * 
 */
class Margulis : public Graph<> {
public:
    /**
     * @brief Construct a new Margulis object
     * 
     * @param nodesNumber 
     */
    explicit Margulis(size_t nodesNumber);
};




// 
/**
 * @brief random weighted graph for usage as e. g. ESN reservoir, TODO implement
 * 
 * @tparam WType 
 * @tparam isDense 
 */
template <typename WType, bool isDense>
class RandomUniform : public Graph<WType, isDense, false> {
public:
    /**
     * @brief Construct a new Random Uniform object
     * 
     * @param nNodes 
     * @param lower_bound 
     * @param upper_bound 
     * @param nConections 
     */
    RandomUniform(size_t nNodes, WType lower_bound = -1, WType upper_bound = -1, int nConections = 0);

private:
    void fill(blaze::CompressedMatrix<WType> & matrix, WType lower_bound, WType upper_bound, int nConections);
    void fill(blaze::DynamicMatrix<WType> & matrix, WType lower_bound, WType upper_bound, int nConections);
    template <typename MType>
    void fill(MType & matrix, WType lower_bound, WType upper_bound);
};



/**
 * @class KNNGraph
 * @brief Fast hierarchial method algorithm that constructs an approximate kNN graph. The method is simple and it works with any type of data for which a distance function can be provided. 
 * Algorith has two parts. In the first part we create a crude approximation of the graph by subdividing the dataset until each subset reaches given max_bruteforce_size.
 * In the second part this approximation is iteratively fine-tuned by combining the first algorithm with NN-descent method. 
 * 
 */
template <typename Sample, typename Distance, typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
class KNNGraph : public Graph<WeightType, isDense, isSymmetric> {
public:

    /**
     * @brief Construct a new KNN Graph object
     * 
     */
    KNNGraph(std::vector<Sample> X, size_t neighbors_num, size_t max_bruteforce_size, int max_iterations = 100, double update_range = 0.02);

    /**
     * @brief Construct a new KNN Graph object
     * 
     */
    KNNGraph(const KNNGraph& graph);

    ///**
    // * @brief Construct a new KNN Graph object
    // * 
    // */
    //KNNGraph(Tree<Sample, Distance>& tree, size_t neighbors_num, size_t max_bruteforce_size, int max_iterations = 100, double update_range = 0.02);

    ///**
    // * @brief Construct a new KNN Graph object
    // * 
    // */
    //KNNGraph(std::vector<std::vector<typename Distance::value_type>> distance_matrix);

	
	size_t neighbors_num() { return _neighbors_num; };

	
	Sample get_node_data(size_t i) { return _nodes[i]; };

	
	std::vector<int> gnnn_search(Sample query, int max_closest_num, int iterations = 10, int num_greedy_moves = -1, int num_expansions = -1);
	
protected:
	size_t _neighbors_num = 1;
	size_t _max_bruteforce_size = 10;
	
	int _max_iterations = 100;
	double _update_range = 0.02;

	bool _not_more_neighbors = false;

	std::vector<Sample> _nodes;
	std::vector<std::vector<typename Distance::value_type>> _distance_matrix;

private:
    /**
     * @brief 
     * 
     */
    void construct(std::vector<Sample> X);

    /**
     * @brief 
     * 
     */
    void calculate_distance_matrix(std::vector<Sample> X);

    /**
     * @brief 
     * 
     */
    void make_edge_pairs(std::vector<Sample> X);
	
    /**
     * @brief 
     * 
     */
	std::vector<std::pair<size_t, size_t>> brute_force(std::vector<Sample> samples, std::vector<int> ids);
	
    /**
     * @brief 
     * 
     */
	std::vector<std::pair<size_t, size_t>> random_pair_division(std::vector<Sample> samples, std::vector<int> ids, int max_size);

    /**
     * @brief 
     * 
     */
	template <typename T1>
	std::vector<size_t> sort_indexes(const std::vector<T1> &v);
};





/**
 * @brief create Graph object based on blaze::CompressedMatrix
 * 
 * @tparam ValueType 
 * @param matrix 
 * @return
 */
template <class ValueType>
Graph<ValueType, false, false> make_graph(blaze::CompressedMatrix<ValueType> && matrix);

/**
 * @brief create Graph object based on blaze::SymetricMatrix<CompressedMatrix>
 * 
 * @tparam ValueType 
 * @param matrix 
 * @return
 */
template <class ValueType>
Graph<ValueType, false, true> make_graph(blaze::SymmetricMatrix<blaze::CompressedMatrix<ValueType>> && matrix);

/**
 * @brief create Graph object based on blaze::DynamicMatrix
 * 
 * @tparam ValueType 
 * @param matrix 
 * @return
 */
template <class ValueType>
Graph<ValueType, true, false> make_graph(blaze::DynamicMatrix<ValueType> && matrix);

/**
 * @brief  create Graph object based on blaze::SymetricMatrix<DynamicMatrix>
 * 
 * @tparam ValueType 
 * @param matrix 
 * @return
 */
template <class ValueType>
Graph<ValueType, true, true> make_graph(blaze::SymmetricMatrix<blaze::DynamicMatrix<ValueType>> && matrix);




} // end namespace metric


#include "graph/graph.cpp"

#endif // header guards
