/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/
#ifndef _METRIC_UTILS_GRAPH_GRAPH_CPP
#define _METRIC_UTILS_GRAPH_GRAPH_CPP
#include "../graph.hpp"
#include <unordered_map>

namespace metric {

// Graph based on blaze-lib

template <typename WeightType, bool isDense, bool isSymmetric>
Graph<WeightType, isDense, isSymmetric>::Graph(size_t nodesNumber)
    : nodesNumber(nodesNumber)
    , valid(false)
{
}

template <typename WeightType, bool isDense, bool isSymmetric>
Graph<WeightType, isDense, isSymmetric>::Graph()
    : nodesNumber(0)
    , valid(false)
{
}

template <typename WeightType, bool isDense, bool isSymmetric>
Graph<WeightType, isDense, isSymmetric>::Graph(const std::vector<std::pair<size_t, size_t>>& edgesPairs)
{
    buildEdges(edgesPairs);
}

template <typename WeightType, bool isDense, bool isSymmetric>
Graph<WeightType, isDense, isSymmetric>::Graph(MatrixType&& matrix)
    : m(std::move(matrix))
{
}

template <typename WeightType, bool isDense, bool isSymmetric>
Graph<WeightType, isDense, isSymmetric>::~Graph() = default;

template <typename WeightType, bool isDense, bool isSymmetric>
size_t Graph<WeightType, isDense, isSymmetric>::getNodesNumber()
{
    return nodesNumber;
}

template <typename WeightType, bool isDense, bool isSymmetric>
bool Graph<WeightType, isDense, isSymmetric>::isValid()
{
    return valid;
}

// left for comparison
template <typename WeightType, bool isDense, bool isSymmetric>
std::vector<std::vector<size_t>> Graph<WeightType, isDense, isSymmetric>::getNeighboursOld(
    const size_t index, const size_t maxDeep)
{
    // return empty if weighted, TODO implement weight-based metric if needed
    if (isWeighted)
        return std::vector<std::vector<size_t>>(0);

    std::vector<std::vector<size_t>> neighboursList(maxDeep + 1);

    std::stack<typename Graph<WeightType, isDense, isSymmetric>::MatrixType::Iterator> iterator_stack;
    std::stack<size_t> row_stack;
    std::unordered_map<size_t, size_t> indices;

    row_stack.push(index);
    iterator_stack.push(m.begin(index));  // stacks are ever processed together (so they are always of the same size)

    indices[row_stack.top()] = 0;

    size_t depth = 1;

    while (true) {
        if (iterator_stack.top() == m.end(row_stack.top()) || depth > maxDeep)  // end of row or max depth reached
        {
            row_stack.pop();  // return to the previous level
            iterator_stack.pop();
            depth--;

            if (depth < 1)  // finish
                break;
        } else {
            row_stack.push(iterator_stack.top()->index());  // enter the next level
            iterator_stack.push(m.begin(row_stack.top()));
            depth++;

            auto search = indices.find(row_stack.top());
            if (search == indices.end()
                || search->second > depth - 1)  // node not exists or its depth is greater than current
            {
                indices[row_stack.top()] = depth - 1;  // write node into output
            }
            continue;  // prevent from a step along the level when entered the new level
        }
        iterator_stack.top()++;  // step along the level
    }

    for (const auto& [index, deep] : indices) {
        neighboursList[deep].push_back(index);
    }

    return neighboursList;
}

template <typename WeightType, bool isDense, bool isSymmetric>
size_t Graph<WeightType, isDense, isSymmetric>::modularPow(
    const size_t base, const size_t exponent, const size_t modulus)
{
    if (modulus == 1) {
        return 1;
    }
    size_t c = 1;
    for (size_t e = 0; e < exponent; ++e) {
        c = (c * base) % modulus;
    }

    return c;
}

template <typename WeightType, bool isDense, bool isSymmetric>
void Graph<WeightType, isDense, isSymmetric>::buildEdges(const std::vector<std::pair<size_t, size_t>>& edgesPairs)
{
    size_t max = 0;
    for (const auto& [i, j] : edgesPairs) {  // TODO loop may be removed if max is passed as a parameter
        if (i > max)
            max = i;
        if (j > max)
            max = j;
    }

    max = max + 1;

    if (max > nodesNumber)
        nodesNumber = max;

    m.resize((unsigned long)max, (unsigned long)max);
    m.reset();

    //m.reserve(edgePairs.size()); // TODO optimize via reserve-insert-finalize idiom
    for (const auto& [i, j] : edgesPairs) {
        if (i != j) {
            m(i, j) = 1;
        }

        // TODO on the final optimization stage: restore faster insertion via .insert(...) with compile-time check/specialization
    }
}

template <typename WeightType, bool isDense, bool isSymmetric>
template <typename T, bool denseFlag>
typename std::enable_if_t<!std::is_same<T, bool>::value, std::vector<std::vector<size_t>>>
Graph<WeightType, isDense, isSymmetric>::getNeighbours(const size_t index, const size_t maxDeep)
{
    // return empty if weighted, TODO implement weight-based metric if needed
    return std::vector<std::vector<size_t>>(0);
}

template <typename WeightType, bool isDense, bool isSymmetric>
template <typename T, bool denseFlag>
typename std::enable_if_t<std::is_same<T, bool>::value && !denseFlag, std::vector<std::vector<size_t>>>
Graph<WeightType, isDense, isSymmetric>::getNeighbours(const size_t index, const size_t maxDeep)
{
    std::vector<std::vector<size_t>> neighboursList(maxDeep + 1);

    if (index >= m.columns())
        return neighboursList;

    std::vector<size_t> parents;
    std::vector<size_t> children;
    std::vector<bool> nodes = std::vector<bool>(m.columns(), false);

    parents.push_back(index);
    neighboursList[0].push_back(index);
    nodes[index] = true;

    size_t depth = 1;

    while (depth <= maxDeep) {
        typename MatrixType::Iterator it;

        for (auto el : parents) {
            for (it = m.begin(el); it != m.end(el);
                 it++)  // for dense and for sparse m.end(...) has different meaning!..
            {
                // tested on sparse only
                if (!nodes[it->index()]) {
                    neighboursList[depth].push_back(it->index());  // write node into output
                    children.push_back(it->index());
                    nodes[it->index()] = true;
                }
            }
        }

        depth++;
        parents.swap(children);
        children = {};
    }

    return neighboursList;
}

template <typename WeightType, bool isDense, bool isSymmetric>
template <typename T, bool denseFlag>
typename std::enable_if_t<std::is_same<T, bool>::value && denseFlag, std::vector<std::vector<size_t>>>
Graph<WeightType, isDense, isSymmetric>::getNeighbours(const size_t index, const size_t maxDeep)
{
    // similar to sparse specialization except the way m elements are accessed
    std::vector<std::vector<size_t>> neighboursList(maxDeep + 1);

    if (index >= m.columns())
        return neighboursList;

    std::vector<size_t> parents;
    std::vector<size_t> children;
    std::vector<bool> nodes = std::vector<bool>(m.columns(), false);

    parents.push_back(index);
    neighboursList[0].push_back(index);
    nodes[index] = true;

    size_t depth = 1;

    while (depth <= maxDeep) {
        typename MatrixType::Iterator it;

        for (auto el : parents) {
            for (it = m.begin(el); it != m.end(el); it++) {
                size_t idx = it - m.begin(el);
                if (m(el, idx) != 1)
                    continue;
                if (!nodes[idx]) {
                    neighboursList[depth].push_back(idx);  // write node into output
                    children.push_back(idx);
                    nodes[idx] = true;
                }
            }
        }

        depth++;
        parents.swap(children);
        children = {};
    }

    return neighboursList;
}

template <typename WeightType, bool isDense, bool isSymmetric>
typename Graph<WeightType, isDense, isSymmetric>::MatrixType Graph<WeightType, isDense, isSymmetric>::get_matrix()
{
    return m;
}

// end of base class implementation

// Grid4_blaze

Grid4::Grid4(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    size_t s = sqrt(nodesNumber);
    if ((s * s) != nodesNumber) {
        valid = false;
    } else {
        construct(s, s);
    }
}

Grid4::Grid4(size_t width, size_t height)
    : Graph<>(width * height)
{
    construct(width, height);
}

void Grid4::construct(size_t width, size_t height)
{
    unsigned long n_nodes = width * height;
    m.resize(n_nodes, n_nodes);

    std::vector<std::pair<size_t, size_t>> edgesPairs;

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {

            int ii0 = 0, ii1 = 0;
            int jj0 = 0, jj1 = 0;

            if (i > 0) {
                ii0 = -1;
            }
            if (j > 0) {
                jj0 = -1;
            }

            if (i < height - 1) {
                ii1 = 1;
            }
            if (j < width - 1) {
                jj1 = 1;
            }

            for (int ii = ii0; ii <= ii1; ++ii) {
                for (int jj = jj0; jj <= jj1; ++jj) {
                    if ((ii == 0) || (jj == 0)) {
                        edgesPairs.emplace_back(i * width + j, (i + ii) * width + (j + jj));
                    }
                }
            }
        }
    }

    buildEdges(edgesPairs);

    valid = true;
}

// Grig6_blaze

Grid6::Grid6(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    size_t s = sqrt(nodesNumber);
    if ((s * s) != nodesNumber) {
        valid = false;
    } else {
        construct(s, s);
    }
}

Grid6::Grid6(size_t width, size_t height)
    : Graph<>(width * height)
{
    construct(width, height);
}

void Grid6::construct(size_t width, size_t height)
{
    unsigned long n_nodes = width * height;
    m.resize(n_nodes, n_nodes);

    std::vector<std::pair<size_t, size_t>> edgesPairs;

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {

            bool odd = true;
            if (i % 2 == 0) {
                odd = false;
            }

            bool up = true;
            if (i == 0) {
                up = false;
            }
            bool down = true;
            if (i == height - 1) {
                down = false;
            }
            bool left = true;
            if (j == 0) {
                left = false;
            }

            bool right = true;
            if (j == width - 1) {
                right = false;
            }

            if (up) {
                edgesPairs.emplace_back(i * width + j, (i - 1) * width + (j + 0));
            }
            if (down) {
                edgesPairs.emplace_back(i * width + j, (i + 1) * width + (j + 0));
            }
            if (left) {
                edgesPairs.emplace_back(i * width + j, (i + 0) * width + (j - 1));
            }
            if (right) {
                edgesPairs.emplace_back(i * width + j, (i + 0) * width + (j + 1));
            }

            if (!odd && left) {
                if (up) {
                    edgesPairs.emplace_back(i * width + j, (i - 1) * width + (j - 1));
                }
                if (down) {
                    edgesPairs.emplace_back(i * width + j, (i + 1) * width + (j - 1));
                }
            }

            if (odd && right) {
                if (up) {
                    edgesPairs.emplace_back(i * width + j, (i - 1) * width + (j + 1));
                }
                if (down)
                    edgesPairs.emplace_back(i * width + j, (i + 1) * width + (j + 1));
            }
        }
    }

    valid = true;

    buildEdges(edgesPairs);
}

// Grid8_blaze

Grid8::Grid8(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    size_t s = sqrt(nodesNumber);
    if ((s * s) != nodesNumber) {
        valid = false;
    } else {
        construct(s, s);
    }
}

Grid8::Grid8(size_t width, size_t height)
    : Graph<>(width * height)
{
    construct(width, height);
}

void Grid8::construct(size_t width, size_t height)
{
    unsigned long n_nodes = width * height;
    m.resize(n_nodes, n_nodes);

    std::vector<std::pair<size_t, size_t>> edgesPairs;

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {

            int ii0 = 0, ii1 = 0;
            int jj0 = 0, jj1 = 0;

            if (i > 0) {
                ii0 = -1;
            }
            if (j > 0) {
                jj0 = -1;
            }

            if (i < height - 1) {
                ii1 = 1;
            }
            if (j < width - 1) {
                jj1 = 1;
            }

            for (int ii = ii0; ii <= ii1; ++ii) {
                for (int jj = jj0; jj <= jj1; ++jj) {
                    if ((ii != 0) || (jj != 0)) {
                        edgesPairs.emplace_back(i * width + j, (i + ii) * width + (j + jj));
                    }
                }
            }
        }
    }

    buildEdges(edgesPairs);

    valid = true;
}

// Paley_blaze

Paley::Paley(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    if (nodesNumber % 4 != 1) {
        return;
    }

    std::vector<std::pair<size_t, size_t>> edgesPairs;

    std::vector<size_t> squareList;

    size_t l = (nodesNumber - 1) / 2;
    squareList.reserve(l);

    for (size_t i = 0; i < l; ++i) {
        squareList.push_back(i * i % nodesNumber);
    }

    for (size_t i = 0; i < nodesNumber; ++i) {
        for (auto j : squareList) {
            edgesPairs.emplace_back(i, (i + j) % nodesNumber);
        }
    }

    buildEdges(edgesPairs);

    valid = true;
}

// LPS_blaze

LPS::LPS(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    if (!millerRabin(nodesNumber)) {
        return;
    }

    std::vector<std::pair<size_t, size_t>> edgesPairs;

    for (size_t i = 0; i < nodesNumber; ++i) {
        if (i == 0) {
            edgesPairs.emplace_back(0, nodesNumber - 1);
            edgesPairs.emplace_back(0, 1);
        } else {
            edgesPairs.emplace_back(i, i - 1);
            edgesPairs.emplace_back(i, (i + 1) % nodesNumber);
            edgesPairs.emplace_back(i, modularPow(i, nodesNumber - 2, nodesNumber));
        }
    }

    buildEdges(edgesPairs);

    valid = true;
}

bool LPS::millerRabin(const size_t nodesNumber)
{
    srand(time(NULL));

    auto d = nodesNumber - 1;
    auto s = 0;

    while (d % 2 == 0) {
        d >>= 1;
        s += 1;
    }

    for (int repeat = 0; repeat < 20; ++repeat) {
        size_t a = 0;
        while (a == 0) {
            a = rand() % nodesNumber;
        }

        if (!miller_rabin_pass(a, s, d, nodesNumber)) {
            return false;
        }
    }
    return true;
}

bool LPS::miller_rabin_pass(const size_t a, const size_t s, const size_t d, const size_t nodesNumber)
{
    auto p = size_t(std::pow(a, d)) % nodesNumber;
    if (p == 1) {
        return true;
    }

    for (size_t i = 0; i < s - 1; ++i) {
        if (p == nodesNumber - 1) {
            return true;
        }
        p = (p * p) % nodesNumber;
    }

    return p == nodesNumber - 1;
}

// Margulis_blaze

Margulis::Margulis(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    size_t s = sqrt(nodesNumber);
    if ((s * s) != nodesNumber) {
        valid = false;
    } else {
        std::vector<std::pair<size_t, size_t>> edgesPairs;

        for (size_t i = 0; i < s; ++i) {
            for (size_t j = 0; j < s; ++j) {
                edgesPairs.emplace_back(i * s + j, ((i + 2 * j) % s) * s + j);
                edgesPairs.emplace_back(i * s + j, i * s + (2 * i + j) % s);
                edgesPairs.emplace_back(i * s + j, i * s + (2 * i + j + 1) % s);
                edgesPairs.emplace_back(i * s + j, ((i + 2 * j + 1) % s) * s + j);
            }
        }

        buildEdges(edgesPairs);

        valid = true;
    }
}

// Random with constant nummber of connections

template <typename WType, bool isDense>
RandomUniform<WType, isDense>::RandomUniform(size_t nNodes, WType lower_bound, WType upper_bound, int nConnections)
    : Graph<WType, isDense, false>(nNodes)
{
    // TODO implement

    using MType = typename std::conditional<isDense, blaze::DynamicMatrix<WType>, blaze::CompressedMatrix<WType>>::type;

    this->m = MType(nNodes, nNodes);
    if (nConnections > 0)
        this->fill(this->m, lower_bound, upper_bound, nConnections);
    else
        this->fill(this->m, lower_bound, upper_bound);

    this->valid = true;
}

template <typename WType, bool isDense>  //
void RandomUniform<WType, isDense>::fill(
    blaze::DynamicMatrix<WType>& matrix, WType lower_bound, WType upper_bound, int nConnections)
{
    std::default_random_engine rgen;
    size_t nNodes = matrix.columns();
    assert(nNodes == matrix.rows());
    auto uniform_int = std::uniform_int_distribution<int>(0, nNodes - 1);
    auto uniform_double = std::uniform_real_distribution<double>(lower_bound, upper_bound);
    int count;
    size_t r_row, r_col;
    for (r_col = 0; r_col < nNodes; r_col++) {
        for (count = 0; count < nConnections; count++) {
            r_row = uniform_int(rgen);
            if (matrix(r_row, r_col)
                == 0)  // TODO profile comparison of doubles, try replacing by lookup in vector of bools
                matrix(r_row, r_col) = uniform_double(rgen);
            else
                count--;  // retry
        }
    }
}

template <typename WType, bool isDense>  // optimized overload for compressed matrix
void RandomUniform<WType, isDense>::fill(
    blaze::CompressedMatrix<WType>& matrix, WType lower_bound, WType upper_bound, int nConnections)
{
    std::default_random_engine rgen;
    size_t nNodes = matrix.columns();
    assert(nNodes == matrix.rows());
    auto uniform_int = std::uniform_int_distribution<int>(0, nNodes - 1);
    auto uniform_double = std::uniform_real_distribution<double>(lower_bound, upper_bound);
    int count;
    size_t r_row, r_col;
    for (r_col = 0; r_col < nNodes; r_col++) {
        for (count = 0; count < nConnections; count++) {
            r_row = uniform_int(rgen);
            if (matrix.find(r_row, r_col) == matrix.end(r_row))  // find works for compressed matrix only
                matrix.insert(r_row, r_col, uniform_double(rgen));
            else
                count--;  // retry
        }
    }
}

template <typename WType, bool isDense>  // total fullfilling for both dense and sparse matrices
template <typename MType>
void RandomUniform<WType, isDense>::fill(MType& matrix, WType lower_bound, WType upper_bound)
{
    std::default_random_engine rgen;
    size_t nNodes = matrix.columns();
    assert(nNodes == matrix.rows());
    auto uniform_double = std::uniform_real_distribution<double>(lower_bound, upper_bound);
    size_t r_row, r_col;
    for (r_row = 0; r_row < nNodes; r_row++)
        for (r_col = 0; r_col < nNodes; r_col++) {
            matrix(r_row, r_col) = uniform_double(rgen);
        }
}

// KNN-Graph

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::KNNGraph(std::vector<Sample> X, size_t neighbors_num, size_t max_bruteforce_size, int max_iterations, double update_range)
    : Graph<WeightType, isDense, isSymmetric>(X.size()), _neighbors_num(neighbors_num), _max_bruteforce_size(max_bruteforce_size), _max_iterations(max_iterations), _update_range(update_range)
{
    construct(X);
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
void KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::construct(std::vector<Sample> samples)
{
    m.resize(samples.size(), samples.size());
	
	std::vector<int> ids(samples.size());
    std::iota(ids.begin(), ids.end(), 0);

	std::vector<std::pair<size_t, size_t>> edgesPairs;
	double updated_percent = 1.0;
	int iterations = 0;
	while(updated_percent > _update_range)
	{
		auto newEdgesPairs = random_pair_division(samples, ids, _max_bruteforce_size);
					
		int was_size = edgesPairs.size();
		for (int j = 0; j < newEdgesPairs.size(); j++)
		{
			bool already_exist = false;

			for (int k = 0; k < edgesPairs.size(); k++)
			{
				if (edgesPairs[k] == newEdgesPairs[j] || (edgesPairs[k].first == newEdgesPairs[j].second && edgesPairs[k].second == newEdgesPairs[j].first))
				{
					already_exist = true;
					break;
				}
			}

			if (!already_exist)
			{
				edgesPairs.push_back(newEdgesPairs[j]);
			}
		}
		if (was_size > 0)
		{
			updated_percent = (double)(edgesPairs.size() - was_size) / was_size;
		}

		iterations++;

		if (iterations >= _max_iterations)
		{
			break;
		}
	}

    buildEdges(edgesPairs);

    valid = true;
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::vector<std::pair<size_t, size_t>> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::random_pair_division(std::vector<Sample> samples, std::vector<int> ids, int max_size)
{
	Distance d;
	std::vector<Sample> A;
	std::vector<Sample> B;
	std::vector<int> A_ids;
	std::vector<int> B_ids;
    std::vector<std::pair<size_t, size_t>> edgesPairs;
    std::vector<std::pair<size_t, size_t>> edgesPairsResult;

	auto n = samples.size();

	if (n > 0)
	{
		std::random_device rnd;
		std::mt19937 mt(rnd());
		std::uniform_int_distribution<int> dist(0, n - 1);

		if (n <= max_size)
		{
			edgesPairs = brute_force(samples, ids);
		}
		else
		{
			auto a = samples[dist(mt)];
			auto b = samples[dist(mt)];
			Sample x;
			for (int i = 0; i < n; i++)
			{
				x = samples[i];
				if (d(x, a) < d(x, b))
				{
					A.push_back(x);
					A_ids.push_back(ids[i]);
				}
				else
				{
					B.push_back(x);
					B_ids.push_back(ids[i]);
				}
			}
			edgesPairsResult = random_pair_division(A, A_ids, max_size);
			edgesPairs.insert( edgesPairs.end(), edgesPairsResult.begin(), edgesPairsResult.end() );
			edgesPairsResult = random_pair_division(B, B_ids, max_size);
			edgesPairs.insert( edgesPairs.end(), edgesPairsResult.begin(), edgesPairsResult.end() );
		}
	}

	return edgesPairs;
}

template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
std::vector<std::pair<size_t, size_t>> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::brute_force(std::vector<Sample> samples, std::vector<int> ids)
{	
    std::vector<std::pair<size_t, size_t>> edgesPairs;

	//
	Distance distance;
	int update_count = 0;
	
    for (int i = 0; i < samples.size(); i++) 
	{
        auto i_point = samples[i];
		std::vector<Distance::value_type> distances;
        for (int j = 0; j < samples.size(); j++) 
		{
            auto i_other_point = samples[j];
            Distance::value_type dist = distance(i_point, i_other_point);
			distances.push_back(dist);
        }

		auto idxs = sort_indexes(distances);

		for (int j = 0; j < idxs.size(); j++)
		{
			// omit first item because it is pair between the same item
			if (j == 0)
			{
				continue;
			}
			
			bool already_exist = false;
			
			std::vector<int>::iterator max_index = std::max_element(ids.begin(), ids.end());
			std::vector<int> sizes(ids[std::distance(ids.begin(), max_index)] + 1, 0);
			for (int k = 0; k < edgesPairs.size(); k++)
			{
				sizes[edgesPairs[k].first]++;
				sizes[edgesPairs[k].second]++;
				if (edgesPairs[k] == std::pair<size_t, size_t>(ids[i], ids[idxs[j]]) || edgesPairs[k] == std::pair<size_t, size_t>(ids[idxs[j]], ids[i]))
				{
					already_exist = true;
					break;
				}
				if (_not_more_neighbors)
				{
					if (sizes[edgesPairs[k].first] >= _neighbors_num || sizes[edgesPairs[k].second] >= _neighbors_num )
					{
						already_exist = true;
						break;
					}
				}
			}
			if (!already_exist)
			{
				edgesPairs.emplace_back(ids[i], ids[idxs[j]]);
			}

			if (j + 1 >= _neighbors_num)
			{
				break;
			}
		}
    }

	return edgesPairs;
}


//unsigned long long nndes_iterate_limited(DataSet* data, kNNGraph* kNN, int _k_limit) {
//
//    vector<vector<int>> new_knn(data->size);
//    vector<vector<int>> old_knn(data->size);
//    vector<vector<int>> reverse_new_knn(data->size);
//    vector<vector<int>> reverse_old_knn(data->size);
//
//    int k_limit = kNN->k;
//    if(_k_limit>0) { k_limit = _k_limit;}
//
//    unsigned long long update_count = 0;
//    g_timer.tuck("  Build old_knn, new_knn ");
//    for(int i_knn = 0; i_knn < kNN->size;i_knn++) {
//        new_knn[i_knn].clear();
//        old_knn[i_knn].clear();
//        reverse_new_knn[i_knn].clear();
//        reverse_old_knn[i_knn].clear();
//
//        for(int j = 0; j < k_limit && j < kNN->list[i_knn].size;j++) {
//            kNNItem* ki = get_kNN_item(kNN,i_knn,j);
//            if(ki->new_item
//                    /*&& (k_limit <= 0 || new_knn[i_knn].size() <= k_limit)*/
//              ) {
//                new_knn[i_knn].push_back(ki->id);
//                ki->new_item = false;
//            } else {
//                old_knn[i_knn].push_back(ki->id);
//            }
//        }
//    }
//
//    // Construct reverse kNN
//    for(int i_knn = 0; i_knn < kNN->size;i_knn++) {
//        for(std::vector<int>::iterator it = new_knn[i_knn].begin(); it != new_knn[i_knn].end(); ++it) {
//            new_knn[*it].push_back(i_knn);
//        }
//        for(std::vector<int>::iterator it = old_knn[i_knn].begin(); it != old_knn[i_knn].end(); ++it) {
//            old_knn[*it].push_back(i_knn);
//        }
//    }
//
//    g_timer.tuck("  START Local join");
//    for(int i_knn = 0; i_knn < kNN->size;i_knn++) {
//
//        std::sort( old_knn[i_knn].begin(), old_knn[i_knn].end());
//        std::sort( new_knn[i_knn].begin(), new_knn[i_knn].end());
//        int last_id = INT_MAX;
//        for(std::vector<int>::iterator it = new_knn[i_knn].begin(); it != new_knn[i_knn].end(); ++it)
//        {
//            int id_A = *it;
//
//            if(id_A == last_id) {continue;}
//            nndes_join(data,kNN,id_A,i_knn,new_knn,update_count);
//            nndes_join_old(data,kNN,id_A,i_knn,old_knn,update_count);
//
//            last_id = id_A;
//        }
//    }
//
//    g_timer.tuck("  END Local join");
//
//    return update_count;
//}


//template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
//std::vector<Sample> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::gnnn_search(Sample query, size_t max_closest_num, size_t iterations, size_t num_greedy_moves, size_t num_expansions)
//{
//	std::vector<Sample> result;
//	std::vector<Distance::value_type> choosen_distances;
//	std::vector<Sample> choosen_points;
//	std::vector<Distance::value_type> distances;
//	Distance::value_type distance;
//
//	Heap<double> pq{knn_graph.query2(k, query_point, mt, e_param, step)};
//	
//	Distance distancer;
//
//	// num_expansions should be less then k(neighbors_num) of the graph
//	if (num_expansions > _neighbors_num)
//	{
//		num_expansions = _neighbors_num;
//	}
//
//	for (int i = 0; i < iterations; i++) 
//	{
//		// get initial node from the graph
//		Y = random;
//		for (int j = 0; j < num_greedy_moves; j++) 
//		{
//			distances.clear();
//			neighbours = Y.neighbours;
//
//			// get first num_expansions neighbours for Y and calculate distances to the query
//			for (int p = 0; p < num_expansions; p++) 
//			{
//				distance = distancer(neighbours[p], query);
//				distances.push_back(distance);
//				choosen_distances.push_back(distances[idxs[q]]);
//				choosen_points.push_back(neighbours[idxs[q]]);
//			}
//			
//			std::vector<int>::iterator min_index = std::min_element(distances.begin(), distances.end());
//			Y = neighbours[std::distance(ids.begin(), max_index)];
//		}
//	}
//			
//	// sort distances and return corresopnding nodes from choosen
//
//	auto idxs = sort_indexes(choosen_distances);
//
//	for (int i = 0; i < max_closest_num; i++)
//	{
//		result.push_back(choosen_points[idxs[i]]);
//	}
//
//	return result;
//}


template <typename Sample, typename Distance, typename WeightType, bool isDense, bool isSymmetric>
template <typename T1>
std::vector<size_t> KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>::sort_indexes(const std::vector<T1> &v) 
{
	// initialize original index locations
	std::vector<size_t> idx(v.size());
	std::iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	std::sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

	return idx;
}

// Graph factory based on Blaze matrix of 4 allowed types

template <class ValueType>
Graph<ValueType, false, false> make_graph(blaze::CompressedMatrix<ValueType>&& matrix)
{
    return Graph<ValueType, false, false>(std::move(matrix));
}

template <class ValueType>
Graph<ValueType, false, true> make_graph(blaze::SymmetricMatrix<blaze::CompressedMatrix<ValueType>>&& matrix)
{
    return Graph<ValueType, false, true>(std::move(matrix));
}

template <class ValueType>
Graph<ValueType, true, false> make_graph(blaze::DynamicMatrix<ValueType>&& matrix)
{
    return Graph<ValueType, true, false>(std::move(matrix));
}

template <class ValueType>
Graph<ValueType, true, true> make_graph(blaze::SymmetricMatrix<blaze::DynamicMatrix<ValueType>>&& matrix)
{
    return Graph<ValueType, true, true>(std::move(matrix));
}

}  // end namespace metric
#endif