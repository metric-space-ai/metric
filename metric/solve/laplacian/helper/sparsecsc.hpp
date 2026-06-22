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

#ifndef _METRIC_UTILS_SOLVER_HELPER_SPARSECSC_HPP
#define _METRIC_UTILS_SOLVER_HELPER_SPARSECSC_HPP

#include <metric/numeric/Math.h>
#include <vector>

namespace mtrc {

/**
 * @class SparseMatrixCSC
 *
 * @brief Julia sparse matrix class
 */
template <typename Tv> class SparseMatrixCSC {
  public:
	/**
	 * @brief Construct a new SparseMatrixCSC object
	 *
	 * @param am
	 * @param an
	 * @param acolptr
	 * @param arowval
	 * @param anzval
	 */
	SparseMatrixCSC(size_t am, size_t an, std::vector<size_t> &acolptr, std::vector<size_t> arowval,
					mtrc::numeric::DynamicVector<Tv> anzval)
		: m(am), n(an), colptr(acolptr), rowval(arowval), nzval(anzval)
	{
	}

	/**
	 * @brief Construct a new SparseMatrixCSC object
	 *
	 */
	SparseMatrixCSC() : m(0), n(0) {}

	/**
	 * @brief Construct a new SparseMatrixCSC from mtrc::numeric::CompressedMatrix
	 *
	 * @param mat
	 */
	SparseMatrixCSC(const mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> &mat);

	/**
	 * @brief Convert to mtrc::numeric::CompressedMatrix
	 *
	 * @return result of conversion this to mtrc::numeric::CompressedMatrix
	 */
	mtrc::numeric::CompressedMatrix<Tv, mtrc::numeric::columnMajor> toCompressedMatrix();

	/**
	 * @brief equality operator
	 *
	 * @param b other sparse matrix
	 * @return true if matricies is equal
	 * @return false if matricies is not equal
	 */
	const bool operator==(const SparseMatrixCSC<Tv> &b) const;

	size_t m;
	size_t n;
	std::vector<size_t> colptr;				  // vector type for size_t
	std::vector<size_t> rowval;				  // vector type for size_t
	mtrc::numeric::DynamicVector<Tv> nzval; // mtrc::numeric::DynamicVector type for Tv type
};
} // namespace mtrc

#include "sparsecsc/sparsecsc.cpp"
#endif
