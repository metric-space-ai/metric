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
#ifndef _METRIC_UTILS_SOLVER_HELPER_GRAPHGALS_CPP
#define _METRIC_UTILS_SOLVER_HELPER_GRAPHGALS_CPP
#include "../ijvstruct.hpp"
#include <fstream>
#include <string>
#include <tuple>
#include <utility>

namespace metric {
template <typename Tv> void matrixToFile(blaze::CompressedMatrix<Tv, blaze::columnMajor> A, std::string FileName)
{
	std::ofstream f(FileName);

	for (size_t i = 0; i < A.rows(); i++) {
		for (size_t j = 0; j < A.columns(); j++) {
			f << A(i, j) << ", ";
		}
		f << std::endl;
	}
}

template <typename Tv> blaze::CompressedMatrix<Tv, blaze::columnMajor> grid2(size_t n, size_t m, Tv isotropy)
{

	blaze::CompressedMatrix<Tv, blaze::columnMajor> res;

	IJV<Tv> ijv = grid2_ijv<Tv>(n, m);

	res = sparse(ijv);

	return std::move(res);
}

template <typename Tv> IJV<Tv> grid2_ijv(size_t n, size_t m, Tv isotropy)
{

	IJV<Tv> isograph = isotropy * path_graph_ijv<Tv>(n);
	IJV<Tv> pgi = path_graph_ijv<Tv>(m);
	IJV<Tv> res = product_graph(isograph, pgi);

	return res;
}
template <typename Tv> inline IJV<Tv> grid2_ijv(size_t n) { return grid2_ijv<Tv>(n, n); }

template <typename Tv> IJV<Tv> product_graph(IJV<Tv> b, IJV<Tv> a)
{

	size_t n = a.n * b.n;

	assert(a.i.size() == a.nnz);

	blaze::DynamicVector<Tv> bncollect(b.n), ancollect(a.n), annzOnes(a.nnz, 1), bnnzOnes(b.nnz, 1), anOnes(a.n, 1),
		bnOnes(b.n, 1);

	for (size_t i = 0; i < b.n; ++i)
		bncollect[i] = (Tv)i;

	for (size_t i = 0; i < a.n; ++i)
		ancollect[i] = (Tv)i;

	blaze::DynamicVector<Tv> ait(a.i.size());

	for (size_t i = 0; i < a.i.size(); ++i) {
		ait[i] = (Tv)a.i[i] + 1;
	}

	blaze::DynamicVector<Tv> ajt(a.j.size());

	for (size_t i = 0; i < a.j.size(); ++i) {
		ajt[i] = (Tv)a.j[i] + 1;
	}

	blaze::DynamicVector<Tv> bit(b.i.size());

	for (size_t i = 0; i < b.i.size(); ++i) {
		bit[i] = (Tv)(b.i[i]);
	}

	blaze::DynamicVector<Tv> bjt(b.j.size());

	for (size_t i = 0; i < b.j.size(); ++i) {
		bjt[i] = (Tv)(b.j[i]);
	}

	blaze::DynamicVector<Tv> a_edge_from = kron<Tv>(annzOnes, (Tv)a.n * bncollect);
	blaze::DynamicVector<Tv> ai = a_edge_from + kron(ait, bnOnes);
	blaze::DynamicVector<Tv> aj = a_edge_from + kron(ajt, bnOnes);
	blaze::DynamicVector<Tv> av = kron(a.v, bnOnes);

	blaze::DynamicVector<Tv> b_edge_from = kron(ancollect, bnnzOnes);
	blaze::DynamicVector<Tv> bi = b_edge_from + kron<Tv>(anOnes, bit * (Tv)a.n);
	blaze::DynamicVector<Tv> bj = b_edge_from + kron<Tv>(anOnes, bjt * (Tv)a.n);
	blaze::DynamicVector<Tv> bv = kron(anOnes, b.v);

	for (size_t i = 0; i < ai.size(); ++i) {
		ai[i] -= 1;
		aj[i] -= 1;
	}

	IJV<Tv> IJVA(n, av.size(), ai, aj, av);
	IJV<Tv> IJVB(n, bv.size(), bi, bj, bv);
	IJV<Tv> IJVRes = IJVA + IJVB;

	return IJVRes;
}

template <typename Tv> IJV<Tv> path_graph_ijv(size_t n)
{
	IJV<Tv> ijv;
	ijv.n = n;
	ijv.nnz = 2 * (n - 1);

	ijv.i.resize(ijv.nnz);
	ijv.j.resize(ijv.nnz);
	ijv.v.resize(ijv.nnz);

	size_t z = 0;

	for (size_t i = 0; i < ijv.n; i++) {
		long long k = i - 1;
		size_t l = i + 1;
		if (k >= 0) {
			ijv.i[z] = i;
			ijv.j[z] = k;
			ijv.v[z] = 1;
			++z;
		}

		if (l < n) {
			ijv.i[z] = i;
			ijv.j[z] = l;
			ijv.v[z] = 1;
			++z;
		}
	}
	return ijv;
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> forceLap(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a)
{

	blaze::CompressedMatrix<Tv, blaze::columnMajor> af;

	if (blaze::min(a) < 0) {
		af = blaze::abs(a);
		af = ClearDiag(af);
	} else if (blaze::sum(blaze::abs(diag(a))) > 0) {
		af = ClearDiag(a);
	} else {
		af = a;
	}

	return Diagonal(sum(af)) - af;
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> lap(blaze::CompressedMatrix<Tv, blaze::columnMajor> A)
{
	blaze::DynamicVector<Tv> ones(A.rows(), 1);
	blaze::DynamicVector<Tv> d = A * ones;
	blaze::CompressedMatrix<Tv, blaze::columnMajor> Dg = Diagonal(d);
	blaze::CompressedMatrix<Tv, blaze::columnMajor> Res = Dg - A;

	return Res;
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor>
wtedEdgeVertexMat(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &mat)
{

	auto [ai, aj, av] = findnz(triu(mat));

	size_t m = ai.size();
	size_t n = mat.rows();

	blaze::DynamicVector<Tv> v = sqrt(av);

	blaze::CompressedMatrix<Tv, blaze::columnMajor> Sparse1, Sparse2, Res;

	std::vector<size_t> cl = collect(0, m);

	Sparse1 = sparse(cl, ai, v, m, n, true);

	Sparse2 = sparse(cl, aj, v, m, n);

	Res = Sparse1 - Sparse2;

	return Res;
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> triu(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A, size_t k)
{

	blaze::CompressedMatrix<Tv, blaze::columnMajor> Res(A.rows(), A.columns());

	Res.reserve(A.nonZeros() / 2);

	for (size_t i = k; i < A.columns(); ++i) {

		for (auto it = A.begin(i); it != A.end(i); it++) {
			size_t j = it->index();

			if (j < i)
				Res.append(j, i, it->value());
			else
				break;
		}
		Res.finalize(i);
	}

	return Res;
}
template <typename Tv> std::pair<Tv, size_t> findmax(const blaze::DynamicVector<Tv> &v)
{

	size_t index = 0;
	Tv maxvalue = v[0];

	for (size_t i = 1; i < v.size(); ++i) {
		Tv curvalue = v[i];

		if (curvalue > maxvalue) {
			maxvalue = curvalue;
			index = i;
		}
	}

	return std::pair<Tv, size_t>(maxvalue, index);
}

template <typename Tv> std::pair<Tv, size_t> findmax(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A, int wise)
{
	size_t index = 0;
	Tv maxvalue;

	if (wise == 1) {
		maxvalue = blaze::max(blaze::row(A, 0));

		for (size_t i = 1; i < A.rows(); i++) {

			Tv curvalue = blaze::max(blaze::row(A, i));

			if (curvalue > maxvalue) {
				maxvalue = curvalue;
				index = i;
			}
		}
	} else if (wise == 2) {
		maxvalue = blaze::max(blaze::column(A, 0));

		for (size_t i = 1; i < A.columns(); i++) {

			Tv curvalue = blaze::max(blaze::column(A, i));

			if (curvalue > maxvalue) {
				maxvalue = curvalue;
				index = i;
			} else
				throw("The value of wise parameter must be 1 or 2.");
		}
	}

	return std::pair<Tv, size_t>(maxvalue, index);
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> sparse(const std::vector<size_t> &I, const std::vector<size_t> &J,
													   const blaze::DynamicVector<Tv> &V, size_t m, size_t n, bool sort)
{
	blaze::CompressedMatrix<Tv, blaze::columnMajor> res(m, n);

	size_t nnz = I.size();

	res.reserve(nnz);

	size_t curcol = 0;
	size_t col = SIZE_MAX;

	if (sort) {

		std::vector<size_t> idx = sortedIndices(I, J, V);

		for (size_t l = 0; l < nnz; ++l) {
			if (J[idx[l]] == col) {
				Tv vv = V[idx[l]];
				res.append(I[idx[l]], J[idx[l]], vv);
			} else {

				for (size_t k = curcol; k < J[idx[l]]; k++) {
					res.finalize(k);
				}
				curcol = col = J[idx[l]];
				Tv vv = V[idx[l]];
				res.append(I[idx[l]], J[idx[l]], vv);
			}
		}
	} else {

		for (size_t l = 0; l < nnz; ++l) {
			if (J[l] == col) {
				Tv vv = V[l];
				res.append(I[l], J[l], vv);
			} else {

				for (size_t k = curcol; k < J[l]; k++) {
					res.finalize(k);
				}
				curcol = col = J[l];
				Tv vv = V[l];
				res.append(I[l], J[l], vv);
			}
		}
	}

	res.finalize(res.columns() - 1);

	return std::move(res);
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> kron(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A,
													 const blaze::CompressedMatrix<Tv, blaze::columnMajor> &B)
{
	blaze::CompressedMatrix<Tv, blaze::columnMajor> Res(A.rows() * B.rows(), A.columns() * B.columns());

	for (size_t i = 0; i < A.columns(); i++) {

		for (auto it = A.cbegin(i); it != A.cend(i); ++it) {
			size_t j = it->index();
			auto sbm = submatrix(Res, j * B.rows(), i * B.columns(), B.rows(), B.columns());

			sbm = A(j, i) * B;
		}
	}

	return Res;
}

template <typename Tv>
inline blaze::DynamicVector<Tv> kron(const blaze::DynamicVector<Tv> &A, const blaze::DynamicVector<Tv> &B)
{
	blaze::DynamicVector<Tv> res(A.size() * B.size());

	for (size_t i = 0; i < A.size(); i++) {
		subvector(res, i * B.size(), B.size()) = A[i] * B;
	}

	return res;
}

template <typename Tv> std::vector<size_t> components(const SparseMatrixCSC<Tv> &mat)
{
	size_t n = mat.n;

	std::vector<size_t> order(n, 0);
	std::vector<size_t> comp(n, 0);

	size_t color = 0;

	for (size_t x = 0; x != n; x++) {
		if (!comp[x]) {		   // not used
			comp[x] = ++color; // insert new color

			if (mat.colptr[x + 1] > mat.colptr[x]) {
				size_t ptr = 0, orderLen = 1;

				order[ptr] = x;

				while (ptr < orderLen) {
					size_t curNode = order[ptr]; // initial curNode=x

					for (size_t ind = mat.colptr[curNode]; ind != mat.colptr[curNode + 1]; ++ind) { // cycle by rows
						size_t nbr = mat.rowval[ind];												// nbr=row
						if (!comp[nbr]) {															// not used
							comp[nbr] = color; // insert current component
							order[orderLen] = nbr;
							++orderLen;
						}
					}
					++ptr;
				}
			}
		}
	}
	return comp;
}

template <typename Tv> std::vector<size_t> components(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &mat)
{
	size_t n = mat.rows();

	std::vector<size_t> order(n, 0);
	std::vector<size_t> comp(n, 0);

	size_t color = 0;

	for (size_t x = 0UL; x < mat.rows(); ++x) {
		if (!comp[x]) {		   // not used
			comp[x] = ++color; // insert new color

			if (mat.begin(x) != mat.end(x)) {
				size_t ptr = 0, orderLen = 1;
				order[ptr] = x;

				while (ptr < orderLen) {
					size_t curNode = order[ptr]; // initial curNode=x

					for (auto it = mat.begin(curNode); it != mat.end(curNode); ++it) {
						size_t nbr = it->index();
						if (!comp[nbr]) {	   // not used
							comp[nbr] = color; // insert current component
							order[orderLen] = nbr;
							++orderLen;
						}
					}
					++ptr;
				}
			}
		}
	}
	return comp;
}

template <typename Tv>
std::tuple<std::vector<size_t>, std::vector<size_t>, blaze::DynamicVector<Tv>>
findnz(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &mat)
{

	size_t nnz = mat.nonZeros();

	std::vector<size_t> i(nnz), j(nnz);
	blaze::DynamicVector<Tv> v(nnz);

	size_t k = 0;

	// Fill i, row and v
	for (size_t l = 0UL; l < mat.rows(); ++l) {

		for (auto it = mat.cbegin(l); it != mat.cend(l); ++it) {

			i[k] = it->index();
			j[k] = l;
			v[k] = it->value();

			++k;
		}
	}

	return std::make_tuple(i, j, v);
}

inline const std::vector<std::vector<size_t>> vecToComps(std::vector<size_t> &compvec)
{

	size_t nc = *max_element(compvec.begin(), compvec.end());

	std::vector<std::vector<size_t>> comps(nc);
	std::vector<size_t> sizes(nc, 0);

	for (size_t i : compvec)
		sizes[i - 1]++;

	for (size_t i = 0; i < nc; i++)
		comps[i].resize(sizes[i]);

	std::vector<size_t> ptrs(nc, 0);

	for (size_t i = 0; i < compvec.size(); i++) {
		size_t c = compvec[i] - 1;

		comps[c][ptrs[c]++] = i;
	}

	return comps;
}

template <typename Tv>
blaze::DynamicVector<Tv> indexbool(const blaze::DynamicVector<Tv> &vect, const std::vector<bool> &idx)
{

	assert(vect.size() == idx.size());

	std::vector<Tv> v;

	for (size_t i = 0; i < idx.size(); i++)
		if (idx[i])
			v.push_back(vect[i]);

	blaze::DynamicVector<Tv> res(v.size());

	for (size_t i = 0; i < v.size(); i++) {
		res[i] = v[i];
	}

	return res;
}
inline std::vector<size_t> indexbool(const std::vector<size_t> &vect, const std::vector<bool> &idx)
{
	assert(vect.size() == idx.size());

	std::vector<size_t> v;

	for (size_t i = 0; i < idx.size(); i++)
		if (idx[i])
			v.push_back(vect[i]);

	return v;
}
template <typename Tv> inline bool testZeroDiag(const Tv &a)
{
	size_t n = a.rows();
	for (size_t i = 0; i < n; i++) {
		if (abs(a(i, i)) > 0)
			return false;
	}

	return true;
}
template <typename Tv> inline blaze::DynamicVector<Tv> dynvec(const std::vector<Tv> &v)
{
	blaze::DynamicVector<Tv> res(v.size());
	for (auto i = 0; i < v.size(); ++i) {
		res[i] = v[i];
	}
	return res;
}

template <typename Tv>
void index(blaze::CompressedMatrix<Tv, blaze::columnMajor> &mout, const std::vector<size_t> &idx, size_t idx2,
		   const blaze::DynamicVector<Tv> &vin)
{

	assert(idx.size() == vin.size());

	mout.reserve(idx2, idx.size());

	for (size_t i = 0; i < idx.size(); i++) {
		mout.append(idx[i], idx2, vin[i]);
	}
	mout.finalize(idx2);
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> index(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A,
													  const std::vector<size_t> &idx1, const std::vector<size_t> &idx2)
{

	blaze::CompressedMatrix<Tv, blaze::columnMajor> Res(idx1.size(), idx2.size(), 0);

	for (size_t j = 0; j < idx2.size(); j++) {
		Res.reserve(j, idx1.size());

		auto curIt = A.cbegin(idx2[j]);
		for (size_t i = 0; i < idx1.size(); ++i) {

			for (auto it = curIt; it != A.cend(idx2[j]); it++) {

				size_t idx = it->index();
				size_t idx_1 = idx1[i];
				if (idx == idx_1) {
					Tv v = it->value();
					Res.append(i, j, v);
					curIt = it;
					break;
				}
			}
		}
		Res.finalize(j);
	}

	return Res;
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> extendMatrix(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a,
															 blaze::DynamicVector<Tv> d)
{
	assert(a.rows() == d.size());

	if (blaze::sum(blaze::abs(d)) == 0)
		return a;

	blaze::DynamicVector<Tv> dpos(d.size() + 1, 0);

	for (size_t i = 0; i < d.size(); i++)
		dpos[i] = d[i] * (d[i] > 0);

	size_t n = d.size();

	blaze::CompressedMatrix<Tv, blaze::columnMajor> Res(a.rows() + 1, a.columns() + 1);

	submatrix(Res, 0, 0, a.rows(), a.columns()) = a;
	row(Res, a.rows()) = trans(dpos);
	column(Res, a.columns()) = dpos;

	return Res;
}

template <typename Tv> blaze::CompressedMatrix<Tv, blaze::columnMajor> Diagonal(const blaze::DynamicVector<Tv> &V)
{

	size_t vsize = V.size();
	blaze::CompressedMatrix<Tv, blaze::columnMajor> Res(vsize, vsize);

	Res.reserve(vsize);

	for (size_t i = 0; i < vsize; i++) {
		Res.append(i, i, V[i]);
		Res.finalize(i);
	}

	return Res;
}

template <typename Tv> std::vector<size_t> flipIndex(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A)
{

	blaze::CompressedMatrix<size_t, blaze::columnMajor> B(A.rows(), A.columns());

	size_t k = 1;
	B.reserve(A.nonZeros());

	for (size_t i = 0; i < A.columns(); i++) {

		for (auto it = A.cbegin(i); it != A.cend(i); ++it) {
			size_t j = it->index();
			B.append(j, i, k);
			++k;
		}
		B.finalize(i);
	}

	B = trans(B);

	std::vector<size_t> resv;
	resv.reserve(B.nonZeros());

	for (size_t i = 0; i < B.columns(); i++) {

		for (auto it = B.cbegin(i); it != B.cend(i); ++it) {
			size_t v = it->value();

			resv.push_back(v - 1);
		}
	}

	return resv;
}
template <typename Tv> inline blaze::CompressedMatrix<Tv, blaze::columnMajor> grid2(size_t n)
{
	if (n < 2)
		throw("Parameter n must more than 2");

	return grid2<Tv>(n, n);
}

template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor>
ClearDiag(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A)
{
	blaze::CompressedMatrix<Tv, blaze::columnMajor> res = A;

	for (size_t i = 0; i < A.rows(); i++)
		res.erase(i, i);

	return res;
}

template <typename Tv>
inline blaze::DynamicVector<Tv> diag(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A, size_t diag_n)
{

	size_t it = A.columns() - diag_n;
	blaze::DynamicVector<Tv> resv(it);

	for (size_t i = 0; i < it; ++i) {
		resv[i] = A(i, diag_n + i);
	}

	return resv;
}
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> adjoint(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A)
{
	return blaze::ctrans(A);
}

template <typename Tv>
inline std::pair<blaze::CompressedMatrix<Tv, blaze::columnMajor>, blaze::DynamicVector<Tv>>
adj(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &sddm)
{
	blaze::CompressedMatrix<Tv, blaze::columnMajor> a = Diagonal(diag(sddm)) - sddm;
	blaze::DynamicVector<Tv> ones(sddm.rows(), 1);
	blaze::DynamicVector<Tv> d = sddm * ones;

	return std::pair<blaze::CompressedMatrix<Tv, blaze::columnMajor>, blaze::DynamicVector<Tv>>(a, d);
}

template <typename Tv>
inline blaze::DynamicVector<Tv> sum(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A, int wise)
{
	blaze::DynamicVector<Tv> res;

	if (wise == 1)
		res = blaze::sum<blaze::rowwise>(A);
	else if (wise == 2)
		res = trans(blaze::sum<blaze::columnwise>(A));
	else
		throw("The value of wise parameter must be 1 or 2.");

	return res;
}

template <typename Tv>
inline blaze::DynamicVector<Tv> index(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A,
									  const std::vector<size_t> &idx1, const size_t idx2)
{
	blaze::DynamicVector<Tv> Res(idx1.size(), 0);

	for (auto it = A.cbegin(idx2); it != A.cend(idx2); it++) {
		size_t i = it->index();
		Res[i] = it->value();
	}

	return Res;
}
template <typename Tv>
inline blaze::DynamicVector<Tv> index(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A, const size_t idx1,
									  const std::vector<size_t> &idx2)
{
	blaze::DynamicVector<Tv> Res(idx2.size(), 0);

	for (size_t i = 0; i < idx2.size(); ++i)
		Res[i] = A(idx1, idx2[i]);

	return Res;
}

template <typename Tv>
inline blaze::DynamicVector<Tv> index(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A,
									  const std::vector<size_t> &idx)
{
	return index(A, idx, 0);
}

template <typename Tv>
inline blaze::DynamicVector<Tv> index(const blaze::DynamicVector<Tv> &vec, const std::vector<size_t> &idx)
{
	blaze::DynamicVector<Tv> Res(idx.size());

	for (size_t i = 0; i < idx.size(); i++) {
		Tv v = vec[idx[i]];
		Res[i] = v;
	}

	return Res;
}

template <typename Tv>
inline void index(blaze::DynamicVector<Tv> &vout, const std::vector<size_t> &idx, const blaze::DynamicVector<Tv> &vin)
{
	assert(idx.size() == vin.size());

	for (size_t i = 0; i < idx.size(); ++i) {
		vout[idx[i]] = vin[i];
	}
}
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> diagmat(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A)
{
	return Diagonal(sum(A));
}

template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> pow(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A,
														   const int n)
{
	blaze::CompressedMatrix<Tv, blaze::columnMajor> res;
	res = A;
	for (size_t i = 0; i < n - 1; i++) {
		res *= A;
	}
	return res;
}

inline std::vector<size_t> collect(size_t start, size_t end)
{
	std::vector<size_t> res(end);

	for (size_t i = 0; i < end; i++)
		res[i] = i;

	return res;
}

template <typename Tv>
inline std::vector<size_t> sortedIndices(const std::vector<size_t> &I, const std::vector<size_t> &J,
										 const blaze::DynamicVector<Tv> &V)
{
	size_t nnz = I.size();
	std::vector<size_t> idcs = collect(0, nnz);

	sort(idcs.begin(), idcs.end(), [&I, &J, &V](size_t idx1, size_t idx2) {
		return J[idx1] < J[idx2] || (J[idx1] == J[idx2] && I[idx1] < I[idx2]);
	});

	return idcs;
}

template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> power(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A,
															 const int k)
{
	blaze::CompressedMatrix<Tv, blaze::columnMajor> ap;
	ap = pow(A, k);
	ap = ClearDiag(ap);
	return ap;
}

template <typename Tv> inline bool isConnected(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &mat)
{
	if (!mat.rows())
		return false;

	std::vector<size_t> cm = components(mat);
	return *max_element(cm.begin(), cm.end()) == 1;
}

template <typename Tv> inline Tv mean(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A)
{
	return blaze::sum(A) / (A.rows() * A.columns());
}

template <typename Tv> inline Tv mean(blaze::DynamicVector<Tv> v) { return blaze::sum(v) / v.size(); }

inline size_t keyMap(size_t x, size_t n) { return x <= n ? x : n + x / n; }
} // namespace metric

#endif
