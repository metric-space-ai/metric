#pragma once
/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

/* 

"Cracker" Algorithm to compute connected components of a graph.

Ref: Fast Connected Components Computation in Large Graphs by Vertex Pruning
Alessandro Lulli, Emanuele Carlini, Patrizio Dazzi, Claudio Lucchese, and Laura Ricci
2016
https://doi.org/10.1109/TPDS.2016.2591038
*/


#include <vector>
#include <list>

#include <iostream>

#include "../../../3rdparty/blaze/Math.h"



template<typename Matrix>
class Cracker {

	typedef typename Matrix::ElementType Ty;

	Matrix PropagationTrees;
	std::list<std::vector<size_t>> Components;
	std::vector<size_t> Seeds;
	size_t ActiveNum;
	std::vector<bool> ActiveVertices;

	void ProcessGraph(Matrix &tempGraph);
	void PropagateTrees();
	void ProcessTreeNode(const size_t node, std::vector<size_t> &Nodevector);
	size_t neighborCount(const size_t n, const blaze::DynamicVector<typename Matrix::ElementType, blaze::rowVector> &neighbors) const;
	size_t incomingCount(const size_t n, Matrix &mx) const;
	void ConvertVertexvectorToMatrix(const std::vector<size_t> &vertices, const Matrix &input, Matrix &output);

public:
	std::vector<Matrix> GetAllComponents(const Matrix &input, const size_t Count);
};

template<typename Matrix>
void Cracker<Matrix>::ProcessGraph(Matrix &tempGraph)
{

	//Min Selection
	Matrix directH(tempGraph.rows(), tempGraph.rows(), 0);

	for (size_t i = 0; i != tempGraph.rows(); ++i) {
		if (ActiveVertices[i]) {
			// An active vertex
			auto neighbors = blaze::row(tempGraph, i);

			for (size_t j = 0; j != neighbors.size(); ++j) {
				if ((ActiveVertices[j] && neighbors[j]) || (j == i)) {
					size_t minVertex = j;
					directH(i, minVertex) = true;

					for (size_t k = 0; k != neighbors.size(); ++k)
						if (ActiveVertices[k] && neighbors[k])
							//Add edge
							directH(k, minVertex) = true;
					break;
				}
			}
		}
	}
	//Pruning 
	tempGraph = 0;

	//Min selection
	for (size_t i = 0; i != directH.rows(); ++i) {
		size_t minVertex = 0;
		if (ActiveVertices[i]) {
			// An active vertex
			auto neighbors = blaze::row(directH, i);
			size_t nbCount = neighborCount(i, neighbors);

			for (size_t j = 0; j != neighbors.size(); ++j) {
				if (ActiveVertices[j] && neighbors[j] && (j != i)) {
					minVertex = j;

					if (nbCount > 1) {
						//Add an edge
						for (size_t k = j + 1; k != neighbors.size(); ++k)
							if (ActiveVertices[k] && neighbors[k] && (k != minVertex)) {

								//Undirect graph, symmetrize
								tempGraph(k, minVertex) = true;
								tempGraph(minVertex, k) = true;
							}
					}
					break;
				}
			}

			size_t cv = incomingCount(i, directH);

			if (!directH(i, i)) {

				//Not connected to itself. Disable the vertex
				ActiveVertices[i] = false;
				--ActiveNum;
				PropagationTrees(minVertex, i) = true;

			} else 
				if (cv == 1 && nbCount == 1) {
					//IsSeed: Connected to itself only, it has not any connected (upcoming and outcoming) vertices 
					Seeds.push_back(i);
					//Disable the vertex
					ActiveVertices[i] = false;
					--ActiveNum;
			}
		}
	}
}

template<typename Matrix>
void Cracker<Matrix>::PropagateTrees() 
{
	size_t seedSize = Seeds.size();
	Components.resize(seedSize);

	auto seedIt = Seeds.begin();

	for (auto &l: Components) {
		ProcessTreeNode(*seedIt, l);
		++seedIt;
	}

	Components.remove_if([](auto &el) {return el.size() == 1; });
}

template<typename Matrix>
void Cracker<Matrix>::ProcessTreeNode(const size_t node, std::vector<size_t> &Nodevector) 
{
	Nodevector.push_back(node);
	auto children = blaze::row(PropagationTrees, node);

	for (size_t i = 0; i != children.size(); ++i)
		if (children[i])
			ProcessTreeNode(i, Nodevector);
}

template<typename Matrix>
size_t Cracker<Matrix>::neighborCount(const size_t n, 
										const blaze::DynamicVector<typename Matrix::ElementType, 
										blaze::rowVector> &neighbors) const 
{
	size_t count = 0;
	for (size_t i = 0; i != neighbors.size(); ++i)
		if (ActiveVertices[i] && neighbors[i]) {
			count++;

			// We compare with 1 only
			if (count == 2) return 2;
		}

	return count;
}

template<typename Matrix>
size_t Cracker<Matrix>::incomingCount(const size_t n, Matrix &mx) const 
{
	typedef typename Matrix::ElementType Ty;
	size_t count = 0;

	for (size_t i = 0; i != mx.rows(); i++)
		if (ActiveVertices[i] && mx(i, n)) {
			++count;
			// We compare with 1 only
			if (count == 2) return 2;
		}

	return count;
}

template<typename Matrix>
void Cracker<Matrix>::ConvertVertexvectorToMatrix(const std::vector<size_t> &vertices, const Matrix &input, Matrix &output)
{
	std::vector<bool>NodesExist(input.rows());

	for (size_t node : vertices)
		NodesExist[node] = true;

	for (size_t i = 0; i < input.rows(); ++i)
		if (NodesExist[i])
			for (size_t j = i + 1; j < input.rows(); ++j)
				if (NodesExist[j]) {
					output(i, j) = input(i, j);
					output(j, i) = input(j, i);
				}
}

template<typename Matrix>
std::vector<Matrix> Cracker<Matrix>::GetAllComponents(const Matrix &input, const size_t Count) 
{
	typedef typename Matrix::ElementType Ty;
	ActiveVertices.assign(input.rows(), true);

	ActiveNum = input.rows();
	PropagationTrees.resize(input.rows(), input.rows());
	PropagationTrees = false;

	Matrix tempG = input;

	do {
		ProcessGraph(tempG);
	} while (ActiveNum > 0);
	 

	PropagateTrees();
	Components.sort([](const auto &x, const auto &y) {return x.size() > y.size(); });

	//if Count = 0 - get all components, 1 - largest
	size_t sz, compsz = Components.size();
	sz = Count ? (Count < compsz ? Count : compsz) : compsz;
	Components.resize(sz);

	std::vector<Matrix> matrices(Components.size(), Matrix(input.rows(), input.rows(), false));

	size_t k = 0;

	for (auto &lst : Components) {
		ConvertVertexvectorToMatrix(lst, input, matrices[k]);
		++k;
	}

	return matrices;
}

// if Count=0 - get all components, 1 - largest
template<typename Matrix>
std::vector<Matrix> connected_components(const Matrix &input, const size_t Count) 
{
	Cracker<Matrix> CA;

	return CA.GetAllComponents(input, Count);
}

// if Count=0 - get all components, 1 - largest
template<typename Matrix>
std::vector<Matrix> all_connected_components(const Matrix &input) {
	Cracker<Matrix> CA;

	return CA.GetAllComponents(input, size_t(0));
}

template<typename Matrix>
std::vector<Matrix> largest_connected_component(const Matrix &input) {
	Cracker<Matrix> CA;

	return CA.GetAllComponents(input, size_t(1));
}