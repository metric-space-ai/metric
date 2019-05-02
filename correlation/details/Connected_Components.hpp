/* This Source Code Form is subject to the terms of the PANDA GmbH
License. You are not allowed to use or edit the code without license. */

#ifndef _CONNECTED_COMPONENTS_HPP
#define _CONNECTED_COMPONENTS_HPP


/* 

"Cracker" Algorithm to compute connected components of a graph.

Ref: Fast Connected Components Computation in Large Graphs by Vertex Pruning
Alessandro Lulli, Emanuele Carlini, Patrizio Dazzi, Claudio Lucchese, and Laura Ricci
2016
https://doi.org/10.1109/TPDS.2016.2591038
*/


#include <vector>
#include <list>


template<typename Matrix>
class Cracker {

	typedef typename Matrix::value_type Ty;

	Matrix PropagationTrees;
	std::list<std::vector<size_t>> Components;
	std::vector<size_t> Seeds;
	size_t ActiveNum;
	std::vector<bool> ActiveVertices;

	void ProcessGraph(Matrix &tempGraph);
	void PropogateTrees();
	void ProcessTreeNode(const size_t node, std::vector<size_t> &Nodevector);
	size_t neighborCount(const size_t n, const std::vector<Ty> &neighbors) const;
	size_t incomingCount(const size_t n, Matrix &mx) const;
	void ConvertVertexvectorToMatrix(const std::vector<size_t> &vertices, const Matrix &input, Matrix &output);

public:
	std::vector<Matrix> GetAllComponents(const Matrix &input, const size_t Count);
};

template<typename Matrix>
void Cracker<Matrix>::ProcessGraph(Matrix &tempGraph) {

	typedef typename Matrix::value_type Ty;	
	//Min Selection
	Matrix directH(tempGraph.size());

	for (size_t i = 0; i != tempGraph.size(); ++i) {
		if (ActiveVertices[i]) {
			// An active vertex
			std::vector<Ty> &neighbors = tempGraph[i];

			for (size_t j = 0; j != neighbors.size(); ++j) {
				if ((ActiveVertices[j] && neighbors[j]) || (j == i)) {
					size_t minVertex = j;
					directH[i][minVertex] = true;

					for (size_t k = 0; k != neighbors.size(); ++k)
						if (ActiveVertices[k] && neighbors[k])
							//Add edge
							directH[k][minVertex] = true;
					break;
				}
			}
		}
	}

	//Pruning 
	tempGraph.reset();

	//Min selection
	for (size_t i = 0; i != directH.size(); ++i) {
		size_t minVertex = 0;
		if (ActiveVertices[i]) {
			// An active vertex
			std::vector<Ty> &neighbors = directH[i];
			size_t nbCount = neighborCount(i, neighbors);

			for (size_t j = 0; j != neighbors.size(); ++j) {
				if (ActiveVertices[j] && neighbors[j] && (j != i)) {
					minVertex = j;

					if (nbCount > 1) {
						//Add an edge
						for (size_t k = j + 1; k != neighbors.size(); ++k)
							if (ActiveVertices[k] && neighbors[k] && (k != minVertex)) {

								//Undirect graph, symmetrize
								tempGraph[k][minVertex] = true;
								tempGraph[minVertex][k] = true;
							}
					}
					break;
				}
			}

			size_t cv = incomingCount(i, directH);

			if (!directH[i][i]) {
				//Not connected to itself. Disable the vertex
				ActiveVertices[i] = false;
				--ActiveNum;
				PropagationTrees[minVertex][i] = true;
			}
			else
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
void Cracker<Matrix>::PropogateTrees() {

	typedef typename Matrix::value_type Ty;
	size_t sz = Seeds.size();
	Components.resize(sz);

	std::vector<size_t>::iterator seedit = Seeds.begin();
	size_t n = 0;

	for (auto &l : Components) {
		size_t seed = *seedit;
		ProcessTreeNode(seed, l);
		++seedit;
		++n;
	}

	Components.remove_if([](auto &el) {return el.size() == 1; });
};

template<typename Matrix>
void Cracker<Matrix>::ProcessTreeNode(const size_t node, std::vector<size_t> &Nodevector) {
	typedef typename Matrix::value_type Ty;
	Nodevector.push_back(node);
	std::vector<Ty> &children = PropagationTrees[node];

	for (size_t i = 0; i != children.size(); ++i)
		if (children[i])
			ProcessTreeNode(i, Nodevector);
}

template<typename Matrix>
size_t Cracker<Matrix>::neighborCount(const size_t n, const std::vector<typename Matrix::value_type> &neighbors) const {
	typedef typename Matrix::value_type Ty;
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
size_t Cracker<Matrix>::incomingCount(const size_t n, Matrix &mx) const {
	typedef typename Matrix::value_type Ty;
	size_t count = 0;

	for (size_t i = 0; i != mx.size(); i++)
		if (ActiveVertices[i] && mx[i][n]) {
			++count;
			// We compare with 1 only
			if (count == 2) return 2;
		}

	return count;
}

template<typename Matrix>
void Cracker<Matrix>::ConvertVertexvectorToMatrix(const std::vector<size_t> &vertices, const Matrix &input, Matrix &output)
{
	typedef typename Matrix::value_type Ty;
	std::vector<bool>NodesExist(input.size());

	for (size_t node : vertices)
		NodesExist[node] = true;

	for (size_t i = 0; i < input.size(); ++i)
		if (NodesExist[i])
			for (size_t j = i + 1; j < input.size(); ++j)
				if (NodesExist[j]) {
					output[i][j] = input[i][j];
					output[j][i] = input[j][i];
				}
}

template<typename Matrix>
std::vector<Matrix> Cracker<Matrix>::GetAllComponents(const Matrix &input, const size_t Count) {
	typedef typename Matrix::value_type Ty;
	ActiveVertices.assign(input.size(), true);

	ActiveNum = input.size();
	PropagationTrees.resize(input.size());

	Matrix tempG = input;

	do {
		ProcessGraph(tempG);
	} while (ActiveNum > 0);

	PropogateTrees();
	Components.sort([](const auto &x, const auto &y) {return x.size() > y.size(); });

	//if Count = 0 - get all components, 1 - largest
	size_t sz, compsz = Components.size();
	sz = Count ? (Count < compsz ? Count : compsz) : compsz;
	Components.resize(sz);

	std::vector<Matrix> matrices(Components.size(), Matrix(input.size()));

	size_t k = 0;

	for (auto &lst : Components) {
		ConvertVertexvectorToMatrix(lst, input, matrices[k]);
		++k;
	}

	return matrices;
}

// if Count=0 - get all components, 1 - largest
template<typename Matrix>
std::vector<Matrix> connected_components(const Matrix &input, const size_t Count) {
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


#endif //headerguard