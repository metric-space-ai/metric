/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Signal Empowering Technology ®Michael Welsch
*/

#ifndef _METRIC_GRAPH_HPP
#define _METRIC_GRAPH_HPP


#include "../../../3rdparty/blaze/Math.h"
#include <stack>
#include <type_traits>


namespace metric {
namespace graph {



// Graph based on blaze-lib


template <typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
class Graph {

private:
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
    explicit Graph(size_t nodesNumber);
    Graph();
    Graph(const std::vector<std::pair<size_t, size_t>> & edgesPairs);
    Graph(MatrixType && matrix);
    ~Graph();

    size_t getNodesNumber();
    bool isValid();

    std::vector<std::vector<size_t>> getNeighboursOld(const size_t nodeIndex, const size_t maxDeep);

    template <typename T = WeightType, bool denseFlag = isDense>
    typename std::enable_if_t<!std::is_same<T, bool>::value, std::vector<std::vector<size_t>>>
    getNeighbours(const size_t nodeIndex, const size_t maxDeep); // not bool

    template <typename T = WeightType, bool denseFlag = isDense>
    typename std::enable_if_t<std::is_same<T, bool>::value && !denseFlag, std::vector<std::vector<size_t>>>
    getNeighbours(const size_t nodeIndex, const size_t maxDeep); // bool, not dense

    template <typename T = WeightType, bool denseFlag = isDense>
    typename std::enable_if_t<std::is_same<T, bool>::value && denseFlag, std::vector<std::vector<size_t>>>
    getNeighbours(const size_t nodeIndex, const size_t maxDeep); // bool, dense

    MatrixType get_matrix();

    void buildEdges(const std::vector<std::pair<size_t, size_t>> &edgesPairs);

protected:
    size_t nodesNumber;
    bool valid;

    MatrixType m;

    size_t modularPow(const size_t base, const size_t exponent, const size_t modulus);

};





class Grid4 : public Graph<> {
public:
    explicit Grid4(size_t nodesNumber);
    Grid4(size_t width, size_t height);
private:
    void construct(size_t width, size_t height);
};




class Grid6 : public Graph<> {
public:
    explicit Grid6(size_t nodesNumber);
    Grid6(size_t width, size_t height);
private:
    void construct(size_t width, size_t height);
};




class Grid8 : public Graph<> {
public:
    explicit Grid8(size_t nodesNumber);
    Grid8(size_t width, size_t height);
private:
    void construct(size_t width, size_t height);
};




class Paley : public Graph<> {
public:
    explicit Paley(size_t nodesNumber);
};




class LPS : public Graph<> {
public:
    explicit LPS(size_t nodesNumber);
private:
    bool miller_rabin_pass(const size_t a, const size_t s,
                           const size_t d, const size_t nodesNumber);

    bool millerRabin(const size_t nodesNumber);
};




class Margulis : public Graph<> {
public:
    explicit Margulis(size_t nodesNumber);
};




// random weighted graph for usage as e. g. ESN reservoir, TODO implement

class Random : public Graph<> {
public:
    explicit Random(size_t nodesNumber, double sparsity);
};





// Graph factory based on Blaze matrix

template <class ValueType>
Graph<ValueType, false, false> make_graph(blaze::CompressedMatrix<ValueType> && matrix);

template <class ValueType>
Graph<ValueType, false, true> make_graph(blaze::SymmetricMatrix<blaze::CompressedMatrix<ValueType>> && matrix);

template <class ValueType>
Graph<ValueType, true, false> make_graph(blaze::DynamicMatrix<ValueType> && matrix);

template <class ValueType>
Graph<ValueType, true, true> make_graph(blaze::SymmetricMatrix<blaze::DynamicMatrix<ValueType>> && matrix);




}
}





#include "graph.cpp"

#endif // header guards
