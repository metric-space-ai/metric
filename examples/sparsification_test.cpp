/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

		Copyright (c) 2019 Panda Team
		Copyright (c) 2019 Oleg Popov
*/
/*
Laplacians is a package containing graph algorithms, with an emphasis on tasks related to 
spectral and algebraic graph theory. It contains (and will contain more) code for solving 
systems of linear equations in graph Laplacians, low stretch spanning trees, sparsifiation, 
clustering, local clustering, and optimization on graphs.

All graphs are represented by sparse adjacency matrices. This is both for speed, and because 
our main concerns are algebraic tasks. It does not handle dynamic graphs. It would be very slow 
to implement dynamic graphs this way.

https://github.com/danspielman/Laplacians.jl
*/

#include <iostream>
#include <blaze/Math.h>
#include <chrono>
#include "sparsification_test.h"
#include "../utils/solver/graphalgs.h"
#include "../utils/graph/sparsify.h"

using namespace std;
using namespace laplacians;

using blaze::CompressedMatrix;


void sparsification_test() {
	
	auto t1 = high_resolution_clock::now();
	CompressedMatrix<double, blaze::columnMajor> G = grid2<double>(100);
	size_t n = G.rows();

	//matrixToFile(G, "grid2.txt");
		
	CompressedMatrix<double, blaze::columnMajor> Gp = power(G, 15);

	//matrixToFile(Gp, "power.txt");

	//cout << "nonzeros before sparsification=" << Gp.nonZeros() << endl;
	CompressedMatrix<double, blaze::columnMajor> Gsparse = sparsify(Gp, 1.0F);
	//matrixToFile(Gsparse, "sparsify.txt");
	//cout << "nonzeros after sparsification=" << Gsparse.nonZeros() << endl;

	cout.precision(10);
	cout << "Average degree of sparsifier: " << double(Gsparse.nonZeros()) / double(n);
	auto t2 = high_resolution_clock::now();
	auto msec = duration_cast<milliseconds>(t2 - t1).count();
	std::cout << "\nFinal time: " << msec / 1000.0 << " ms.\n";
	   	 	
}