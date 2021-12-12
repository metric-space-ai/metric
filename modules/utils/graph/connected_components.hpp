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
#ifndef _METRIC_UTILS_GRAPH_CONNECTED_COMPONENTS_HPP
#define _METRIC_UTILS_GRAPH_CONNECTED_COMPONENTS_HPP

#include <vector>
#include <list>

#include <blaze/Math.h>

namespace metric {
namespace graph {
    /**
     * @class Cracker
     * 
     * @brief
     */
    template <typename Matrix>
    class Cracker {
    public:
        /**
         * @brief Get the All Components
         * 
         * @param input 
         * @param Count 
         * @return 
         */
        std::vector<Matrix> GetAllComponents(const Matrix& input, const size_t Count);
    
    private:
        typedef typename Matrix::ElementType Ty;
        void ProcessGraph(Matrix& tempGraph);
        void PropagateTrees();
        void ProcessTreeNode(const size_t node, std::vector<size_t>& Nodevector);
        size_t neighborCount(const size_t n,
            const blaze::DynamicVector<typename Matrix::ElementType, blaze::rowVector>& neighbors) const;
        size_t incomingCount(const size_t n, Matrix& mx) const;
        void ConvertVertexvectorToMatrix(const std::vector<size_t>& vertices, const Matrix& input, Matrix& output);

        Matrix PropagationTrees;
        std::list<std::vector<size_t>> Components;
        std::vector<size_t> Seeds;
        size_t ActiveNum;
        std::vector<bool> ActiveVertices;

    };


    /**
     * @brief 
     * 
     * @param input 
     * @param Count  if Count=0 - get all components, 1 - largest
     * @return 
     */
    template <typename Matrix>
    std::vector<Matrix> connected_components(const Matrix& input, const size_t Count);

    /**
     * @brief 
     * 
     * @param input 
     * @return
     */
    template <typename Matrix>
    std::vector<Matrix> all_connected_components(const Matrix& input);

    /**
     * @brief 
     * 
     * @param input 
     * @return
     */
    template <typename Matrix>
    std::vector<Matrix> largest_connected_component(const Matrix& input);

}  // namespace graph
}  // namespace metric
#include "connected_components.cpp"
#endif
