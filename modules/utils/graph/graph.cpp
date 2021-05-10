/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/
#include "modules/utils/graph.hpp"

#include <assert.h>
#include <unordered_map>
#include <algorithm>

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

// FIXME: check what to do with other fields
template <typename WeightType, bool isDense, bool isSymmetric>
Graph<WeightType, isDense, isSymmetric>::Graph(MatrixType&& matrix)
    : matrix(std::move(matrix))
{
}

template <typename WeightType, bool isDense, bool isSymmetric>
size_t Graph<WeightType, isDense, isSymmetric>::getNodesNumber() const
{
    return nodesNumber;
}

template <typename WeightType, bool isDense, bool isSymmetric>
bool Graph<WeightType, isDense, isSymmetric>::isValid() const
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
    iterator_stack.push(matrix.begin(index));  // stacks are ever processed together (so they are always of the same size)

    indices[row_stack.top()] = 0;

    size_t depth = 1;

    while (true) {
        if (iterator_stack.top() == matrix.end(row_stack.top()) || depth > maxDeep)  // end of row or max depth reached
        {
            row_stack.pop();  // return to the previous level
            iterator_stack.pop();
            depth--;

            if (depth < 1)  // finish
                break;
        } else {
            row_stack.push(iterator_stack.top()->index());  // enter the next level
            iterator_stack.push(matrix.begin(row_stack.top()));
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

    matrix.resize((unsigned long)max, (unsigned long)max);
    matrix.reset();

    //matrix.reserve(edgePairs.size()); // TODO optimize via reserve-insert-finalize idiom
    for (const auto& [i, j] : edgesPairs) {
        if (i != j) {
            matrix(i, j) = 1;
        }

        // TODO on the final optimization stage: restore faster insertion via .insert(...) with compile-time check/specialization
    }
}

template <typename WeightType, bool isDense, bool isSymmetric>
void Graph<WeightType, isDense, isSymmetric>::updateEdges(const MatrixType &edgeMatrix)
{
    assert (matrix.rows() == edgeMatrix.rows());
    assert (matrix.columns() == edgeMatrix.columns());
	matrix = edgeMatrix;
	matrix_changed_ = true;
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

    if (index >= matrix.columns())
        return neighboursList;

    std::vector<size_t> parents;
    std::vector<size_t> children;
    std::vector<bool> nodes = std::vector<bool>(matrix.columns(), false);

    parents.push_back(index);
    neighboursList[0].push_back(index);
    nodes[index] = true;

    size_t depth = 1;

    while (depth <= maxDeep) {
        typename MatrixType::Iterator it;

        for (auto el : parents) {
            for (it = matrix.begin(el); it != matrix.end(el);
                 it++)  // for dense and for sparse matrix.end(...) has different meaning!..
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
    // similar to sparse specialization except the way matrix elements are accessed
    std::vector<std::vector<size_t>> neighboursList(maxDeep + 1);

    if (index >= matrix.columns())
        return neighboursList;

    std::vector<size_t> parents;
    std::vector<size_t> children;
    std::vector<bool> nodes = std::vector<bool>(matrix.columns(), false);

    parents.push_back(index);
    neighboursList[0].push_back(index);
    nodes[index] = true;

    size_t depth = 1;

    while (depth <= maxDeep) {
        typename MatrixType::Iterator it;

        for (auto el : parents) {
            for (it = matrix.begin(el); it != matrix.end(el); it++) {
                size_t idx = it - matrix.begin(el);
                if (matrix(el, idx) != 1)
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
auto Graph<WeightType, isDense, isSymmetric>::get_matrix() const
    -> const typename Graph<WeightType, isDense, isSymmetric>::MatrixType&
{
    return matrix;
}

// end of base class implementation

// Grid4_blaze

inline Grid4::Grid4(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    auto s = static_cast<size_t>(std::sqrt(nodesNumber));
    if ((s * s) != nodesNumber) {
        valid = false;
    } else {
        construct(s, s);
    }
}

inline Grid4::Grid4(size_t width, size_t height)
    : Graph<>(width * height)
{
    construct(width, height);
}

inline void Grid4::construct(size_t width, size_t height)
{
    size_t n_nodes = width * height;
    matrix.resize(n_nodes, n_nodes);

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

inline Grid6::Grid6(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
	auto s = static_cast<size_t>(std::sqrt(nodesNumber));
    if ((s * s) != nodesNumber) {
        valid = false;
    } else {
        construct(s, s);
    }
}

inline Grid6::Grid6(size_t width, size_t height)
    : Graph<>(width * height)
{
    construct(width, height);
}

inline void Grid6::construct(size_t width, size_t height)
{
    size_t n_nodes = width * height;
    matrix.resize(n_nodes, n_nodes);

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

inline Grid8::Grid8(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    auto s = static_cast<size_t>(sqrt(nodesNumber));
    if ((s * s) != nodesNumber) {
        valid = false;
    } else {
        construct(s, s);
    }
}

inline Grid8::Grid8(size_t width, size_t height)
    : Graph<>(width * height)
{
    construct(width, height);
}

inline void Grid8::construct(size_t width, size_t height)
{
    size_t n_nodes = width * height;
    matrix.resize(n_nodes, n_nodes);

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

inline Paley::Paley(size_t nodesNumber)
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

inline LPS::LPS(size_t nodesNumber)
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

inline bool LPS::millerRabin(const size_t nodesNumber)
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

inline bool LPS::miller_rabin_pass(const size_t a, const size_t s, const size_t d, const size_t nodesNumber)
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

inline Margulis::Margulis(size_t nodesNumber)
    : Graph<>(nodesNumber)
{
    auto s = static_cast<size_t>(sqrt(nodesNumber));
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

    this->matrix = MType(nNodes, nNodes);
    if (nConnections > 0)
        this->fill(this->matrix, lower_bound, upper_bound, nConnections);
    else
        this->fill(this->matrix, lower_bound, upper_bound);

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
