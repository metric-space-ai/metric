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
//#include <boost/container_hash/hash.hpp>
#include "sparsecsc.h"
#include "graphalgs.h"


using namespace std;

using blaze::CompressedMatrix;
using blaze::DynamicMatrix;
using blaze::DynamicVector;

namespace laplacians {

	template <typename Tv>
	class IJV {
	public:
		std::size_t  n; // dimension
		std::size_t nnz; // number of nonzero elements
		vector<std::size_t> i; // row
		vector<std::size_t> j; // col
		DynamicVector<Tv> v; //nonzero elements

		IJV() :n(0), nnz(0), i(0), j(0), v(0) {}

		IJV operator+(const IJV& b) const;

		bool operator== (const IJV &b) const
		{
			bool res = n == b.n &&	nnz == b.nnz &&	i == b.i &&	j == b.j &&	v == b.v;
			return res;
		}

		IJV operator* (const Tv x) const
		{
			IJV m;

			m.n = n; m.nnz = nnz;
			m.i = i; m.j = j;

			m.v = v * x;

			return m;
		}

		IJV(const IJV &a) {
			n = a.n; nnz = a.nnz;
		
			i = a.i; j = a.j; v = a.v; 		
		}

		IJV(const size_t an, const size_t annz,	const vector<size_t> &ai, const vector<size_t> &aj, const DynamicVector<Tv> &av) {

			n = an;	nnz = annz;

			i = ai;	j = aj;	v = av;
		}

		IJV(const size_t  an, const size_t annz, const DynamicVector<size_t> &ai, const DynamicVector<size_t> &aj,
			const DynamicVector<Tv> &av) {

			n = an;	nnz = annz;

			i.insert(i.begin(), ai.begin(), ai.end());
			j.insert(j.begin(), aj.begin(), aj.end());
			v = av;
		}

		const IJV& operator=(const IJV &a) {
			n = a.n; nnz = a.nnz;
			i = a.i; j = a.j; v = a.v;

			return *this;
		}

		IJV(const CompressedMatrix<Tv, blaze::columnMajor>& mat);
		IJV(const SparseMatrixCSC<Tv>& cscm);

		// Convert to blaze::CompressedMatrix
		CompressedMatrix<Tv, blaze::columnMajor> toCompressedMatrix() const {
			return sparse(i, j, v, n, n, true);
		}

		void dump_ijv(int ijvn) const;

		vector<size_t> sortedIndices() const {
			return laplacians::sortedIndices(i, j, v);
		}

		void sortByCol();
	};

	template <typename Tv>
	IJV<Tv> operator* (const Tv x, const IJV<Tv> &ijv) {

		return ijv * x;
	}

	template <typename Tv>
	const std::size_t nnz(const IJV<Tv> &a) {
		return a.nnz;
	}

	/*template <typename Tv>
	size_t hashijv(const IJV<Tv> &a) {
		size_t seed = boost::hash_range(begin(a.v), end(a.v));
		size_t seed1 = boost::hash_range(begin(a.i), end(a.i));
		size_t seed2 = boost::hash_range(begin(a.j), end(a.j));

		boost::hash_combine(seed, seed1);
		boost::hash_combine(seed, seed2);
		boost::hash_combine(seed, a.n);
		boost::hash_combine(seed, a.nnz);

		return seed;
	}
	
	template <typename Tv>
	size_t hashijv(const IJV<Tv> &a, const unsigned h) {
		size_t seed = hashijv(a);
		boost::hash_combine(seed, h);
		return seed;
	}
	*/

	//sparse:  convert IJV to SparseMatrixCSC
	template <typename Tv>
	SparseMatrixCSC<Tv> sparseCSC(const IJV<Tv>& ijv);

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> sparse(const IJV<Tv>& ijv) {
		return move(ijv.toCompressedMatrix());
	}
	
	template <typename Tv>
	IJV<Tv> compress(const IJV<Tv> &ijv) {
		return IJV<Tv>(sparseCSC(ijv));
	}

	template <typename Tv>
	IJV<Tv> transpose(const IJV<Tv> &ijv) {
		return IJV<Tv>(ijv.n, ijv.nnz, ijv.j, ijv.i, ijv.v);
	}
}

#include "ijvstruct.cpp"