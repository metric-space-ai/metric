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

#pragma once

#include <blaze/Math.h>
#include <vector>

using namespace std;
using blaze::CompressedMatrix;
using blaze::DynamicMatrix;
using blaze::DynamicVector;

namespace laplacians {

	// Julia sparse matrix class
	template <typename Tv>
	class SparseMatrixCSC {
	public:
		size_t m;
		size_t n;
		vector<size_t> colptr; // vector type for size_t
		vector<size_t> rowval; // vector type for size_t
		DynamicVector<Tv> nzval; // DynamicVector type for Tv type

		SparseMatrixCSC(size_t am, size_t an, vector<size_t> &acolptr, vector<size_t> arowval, DynamicVector<Tv> anzval) :
			m(am), n(an), colptr(acolptr), rowval(arowval), nzval(anzval) {}

		SparseMatrixCSC() :m(0), n(0) {}

		//Convert from blaze::CompressedMatrix
		SparseMatrixCSC(const CompressedMatrix<Tv, blaze::columnMajor>& mat);

		// Convert to blaze::CompressedMatrix
		CompressedMatrix<Tv, blaze::columnMajor> toCompressedMatrix();

		const bool operator== (const SparseMatrixCSC<Tv> &b) const {
			return 	m == b.m && n == b.n && colptr == b.colptr && rowval == b.rowval && nzval == b.nzval;
		}
	};
}

#include "sparsecsc.cpp"