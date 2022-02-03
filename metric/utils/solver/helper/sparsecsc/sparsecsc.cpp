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
#ifndef _METRIC_UTILS_SOLVER_HELPER_SPARSECSC_CPP
#define _METRIC_UTILS_SOLVER_HELPER_SPARSECSC_CPP
namespace metric {
template <typename Tv> SparseMatrixCSC<Tv>::SparseMatrixCSC(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &mat)
{

	m = mat.rows();
	n = mat.columns();

	size_t nnz = mat.nonZeros();

	colptr.resize(n + 1);
	colptr[0] = 0;
	colptr[n] = nnz;

	rowval.resize(nnz);
	nzval.resize(nnz);

	size_t k = 0;

	// Fill colptr, rowval and nzval

	std::size_t totalnz = 0;

	for (size_t l = 0UL; l < mat.columns(); ++l) {
		std::size_t rownz = 0;

		for (auto it = mat.cbegin(l); it != mat.cend(l); ++it) {

			nzval[k] = it->value();
			rowval[k] = it->index();
			++k;
			++rownz;
		}

		totalnz += rownz;
		colptr[l + 1] = totalnz;
	}
}

template <typename Tv> blaze::CompressedMatrix<Tv, blaze::columnMajor> SparseMatrixCSC<Tv>::toCompressedMatrix()
{
	blaze::CompressedMatrix<Tv, blaze::columnMajor> res(n, n);

	size_t nnz = nzval.size();
	res.reserve(nnz);

	for (size_t i = 0; i != n; i++) {
		size_t colbegin = colptr[i];
		size_t colend = colptr[i + 1];

		for (size_t row = colbegin; row != colend; row++) {
			size_t rowv = rowval[row];
			Tv v = nzval[row];
			res.append(rowv, i, v);
		}
		res.finalize(i);
	}
	return res;
}

template <typename Tv> const bool SparseMatrixCSC<Tv>::operator==(const SparseMatrixCSC<Tv> &b) const
{
	return m == b.m && n == b.n && colptr == b.colptr && rowval == b.rowval && nzval == b.nzval;
}

} // namespace metric
#endif