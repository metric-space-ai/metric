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
#ifndef _METRIC_CORRELATION_DETAILS_CONNECTED_COMPONENTS_HPP
#define _METRIC_CORRELATION_DETAILS_CONNECTED_COMPONENTS_HPP

#include <vector>
#include <list>

#include <iostream>

#include "../../../3rdparty/blaze/Math.h"

namespace metric {
namespace correlation {
template <typename Matrix> class Cracker {

  typedef typename Matrix::ElementType Ty;

  Matrix PropagationTrees;
  std::list<std::vector<size_t>> Components;
  std::vector<size_t> Seeds;
  size_t ActiveNum;
  std::vector<bool> ActiveVertices;

  void ProcessGraph(Matrix &tempGraph);
  void PropagateTrees();
  void ProcessTreeNode(const size_t node, std::vector<size_t> &Nodevector);
  size_t neighborCount(
      const size_t n,
      const blaze::DynamicVector<typename Matrix::ElementType, blaze::rowVector>
          &neighbors) const;
  size_t incomingCount(const size_t n, Matrix &mx) const;
  void ConvertVertexvectorToMatrix(const std::vector<size_t> &vertices,
                                   const Matrix &input, Matrix &output);

public:
  std::vector<Matrix> GetAllComponents(const Matrix &input, const size_t Count);
};

// if Count=0 - get all components, 1 - largest
template <typename Matrix>
std::vector<Matrix> connected_components(const Matrix &input,
                                         const size_t Count);

// if Count=0 - get all components, 1 - largest
template <typename Matrix>
std::vector<Matrix> all_connected_components(const Matrix &input);

template <typename Matrix>
std::vector<Matrix> largest_connected_component(const Matrix &input);

} // namespace correlation
} // namespace metric
#include "connected-components.cpp"
#endif
