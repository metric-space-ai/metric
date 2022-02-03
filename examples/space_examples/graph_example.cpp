/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <iostream>
#include "metric/utils/graph.hpp"

std::vector<std::pair<size_t, size_t>> createGrid4(size_t width, size_t height)
{
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
    return edgesPairs;
}

using VType = bool;
//using VType = float;

//using MType = blaze::DynamicMatrix<VType>;
//using MType = blaze::CompressedMatrix<VType>;
using MType = blaze::SymmetricMatrix<blaze::DynamicMatrix<VType>>;
//using MType = blaze::SymmetricMatrix<blaze::CompressedMatrix<VType>>;

MType createGrid4Matrix(const std::vector<std::pair<size_t, size_t>>& edgesPairs)
{
    MType m;
    size_t max = 0;
    for (const auto& [i, j] : edgesPairs) {
        if (i > max)
            max = i;
        if (j > max)
            max = j;
    }

    max = max + 1;

    m.resize((unsigned long)max, (unsigned long)max);
    m.reset();

    for (const auto& [i, j] : edgesPairs) {
        if (i != j) {
            m(i, j) = 1;
        }
    }

    return m;
}

int main()
{
	std::cout << "Graph space example have started" << std::endl;
	std::cout << "" << std::endl;

    size_t w = 3;  //15;
    size_t h = 3;  //15;
    size_t node = 1;
    size_t max_depth = 4;
	
    auto g = metric::Grid4(h, w);
    // auto g = metric::Grid6(h, w);
    // auto g = metric::Grid8(h, w);
    // auto g = metric::Margulis(h*w);
    // auto g = metric::Paley(h*w);
    // auto g = metric::LPS(h*w); // TODO FIX: no nodes in graph

    std::cout << "based on stack:\n";

    std::vector<std::vector<size_t>> neighbors = g.getNeighboursOld(node, max_depth);
    for (size_t i = 0; i < neighbors.size(); i++)
        for (size_t j = 0; j < neighbors[i].size(); j++)
            std::cout << i << " | " << neighbors[i][j] << "\n";
    std::cout << std::endl;

    std::cout << "based on vector swap:\n";

    std::vector<std::vector<size_t>> neighborsNew = g.getNeighbours(node, max_depth);
    for (size_t i = 0; i < neighborsNew.size(); i++)
        for (size_t j = 0; j < neighborsNew[i].size(); j++)
            std::cout << i << " | " << neighborsNew[i][j] << "\n";

    // custom graph

    std::vector<std::pair<size_t, size_t>> edges = createGrid4(h, w);

    auto g_custom = metric::Graph<bool, true, false>(edges);  // edge value type = bool, isDense = true, isSymmetric = false

    std::cout << "\ncustom graph:\n";
    std::cout << g_custom.get_matrix() << "\n";

    std::vector<std::vector<size_t>> neighborsNewCustom = g_custom.getNeighbours(node, max_depth);
    for (size_t i = 0; i < neighborsNewCustom.size(); i++)
        for (size_t j = 0; j < neighborsNewCustom[i].size(); j++)
            std::cout << i << " | " << neighborsNewCustom[i][j] << "\n";

    // testing factory

    std::cout << "\ntesting factory\n";

    auto m = createGrid4Matrix(edges);
    auto g_custom_m = metric::make_graph(std::move(m));
    std::cout << g_custom_m.get_matrix() << "\n";

    return 0;
}
