/*
                This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed with this
                file, You can obtain one at http://mozilla.org/MPL/2.0/.

                Copyright (c) 2019 Panda Team
                Copyright (c) 2019 Oleg Popov
*/
/*
Laplacians is a package containing graph algorithms, with an emphasis on tasks
related to spectral and algebraic graph theory. It contains (and will contain
more) code for solving systems of linear equations in graph Laplacians, low
stretch spanning trees, sparsifiation, clustering, local clustering, and
optimization on graphs.

All graphs are represented by sparse adjacency matrices. This is both for speed,
and because our main concerns are algebraic tasks. It does not handle dynamic
graphs. It would be very slow to implement dynamic graphs this way.

https://github.com/danspielman/Laplacians.jl
*/

//#include <blaze/Math.h>
#include "metric/utils/graph/sparsify.hpp"
#include "metric/utils/solver/helper/graphalgs.hpp"
#include "metric/distance.hpp"
#include <chrono>
#include <iostream>

using blaze::CompressedMatrix;

void sparsification_test()
{

    auto t1 = std::chrono::high_resolution_clock::now();
    blaze::CompressedMatrix<double, blaze::columnMajor> G = metric::grid2<double>(100);
    size_t n = G.rows();

    blaze::CompressedMatrix<double, blaze::columnMajor> Gp = metric::power(G, 15);

    blaze::CompressedMatrix<double, blaze::columnMajor> Gsparse = metric::sparsify_effective_resistance(Gp, 1.0F);

    std::cout << "Dimension:" << Gp.rows() << "\n";

    std::cout.precision(10);
    std::cout << "Average degree of sparsifier: " << double(Gsparse.nonZeros()) / double(n);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "\nFinal time: " << msec / 1000.0 << " ms.\n";
}
