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

#include <iostream>
#include <blaze/Math.h>
#include <blaze/math/Submatrix.h>
#include <blaze/math/Subvector.h>
#include <vector>
#include <functional>
#include <random>
#include <fstream> 
#include "sparsecsc.h"
#include "ijvstruct.h"

using namespace std;

using blaze::CompressedMatrix;
using blaze::CompressedVector;
using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::columnwise;
using blaze::rowwise;

namespace laplacians {

	template<typename Tv>
	class Random {
	private:
		random_device rd;
		mt19937 gen;
	public:

		Tv rand0_1() {
			return generate_canonical<Tv, 8 * sizeof(Tv)>(gen);
		}

		Tv randn() {
			return rand0_1() * 3.4 - 1.7;
		}

		DynamicVector<Tv> randv(size_t sz) {
			DynamicVector<Tv> res(sz);

			for (size_t i = 0; i < sz; i++)
				res[i] = rand0_1();

			return res;
		}

		Random() :gen(rd()) {}
	};

	template <typename Tv>
	inline CompressedMatrix<Tv, blaze::columnMajor> ClearDiag(const CompressedMatrix<Tv, blaze::columnMajor>& A) {
		CompressedMatrix<Tv, blaze::columnMajor> res = A;

		for (size_t i = 0; i < A.rows(); i++)
			res.erase(i, i);

		return res;
	}

	template<typename Tv>
	vector<size_t> flipIndex(const CompressedMatrix<Tv, blaze::columnMajor>& A);

	template <typename Tv>
	inline DynamicVector<Tv> diag(const CompressedMatrix<Tv, blaze::columnMajor>& A, size_t diag_n = 0) {

		size_t it = A.columns() - diag_n;
		DynamicVector<Tv> resv(it);

		for (size_t i = 0; i < it; ++i) {
			resv[i] = A(i, diag_n + i);
		}

		return resv;
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> Diagonal(const DynamicVector<Tv>& V);


	//Conjugate transpose
	template <typename Tv>
	inline CompressedMatrix<Tv, blaze::columnMajor> adjoint(const CompressedMatrix<Tv, blaze::columnMajor>& A) {
		CompressedMatrix<Tv, blaze::columnMajor> Res = blaze::ctrans(A);

		return Res;
	}

	/*
	Create an adjacency matrix and a diagonal vector from an SDDM-matrix.
	That is, from a Laplacian with added diagonal weights
	*/

	template <typename Tv>
	inline pair<CompressedMatrix<Tv, blaze::columnMajor>, DynamicVector<Tv>> adj(const CompressedMatrix<Tv, blaze::columnMajor>& sddm)
	{
		CompressedMatrix<Tv, blaze::columnMajor> a = Diagonal(diag(sddm)) - sddm;
		DynamicVector<Tv> ones(sddm.rows(), 1);
		DynamicVector<Tv> d = sddm * ones;

		return pair<CompressedMatrix<Tv, blaze::columnMajor>, DynamicVector<Tv>>(a, d);
	}

	// Add a new vertex to a with weights to the other vertices corresponding to diagonal surplus weight.
	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> extendMatrix(const CompressedMatrix<Tv, blaze::columnMajor>& a,
		DynamicVector<Tv> d);

	template <typename Tv>
	inline DynamicVector<Tv> sum(const CompressedMatrix<Tv, blaze::columnMajor>& A, int wise = 1) {
		DynamicVector<Tv> res;

		if (wise == 1)
			res = blaze::sum<rowwise>(A);
		else
			if (wise == 2)
				res = trans(blaze::sum<columnwise>(A));
			else throw("The value of wise parameter must be 1 or 2.");

		return res;
	}

	// vector index of matrix
	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> index(const CompressedMatrix<Tv, blaze::columnMajor>& A,
		const vector<size_t>& idx1, const vector<size_t>& idx2);

	// vector index of matrix
	template <typename Tv>
	inline DynamicVector<Tv> index(const CompressedMatrix<Tv, blaze::columnMajor>& A, const vector<size_t>& idx1, const size_t idx2) {
		DynamicVector<Tv> Res(idx1.size(), 0);

		for (auto it = A.cbegin(idx2); it != A.cend(idx2); it++)
		{
			size_t i = it->index();
			Res[i] = it->value();
		}

		return Res;
	}

	template <typename Tv>
	inline DynamicVector<Tv> index(const CompressedMatrix<Tv, blaze::columnMajor>& A, const size_t idx1, const vector<size_t>& idx2) {
		DynamicVector<Tv> Res(idx2.size(), 0);

		for (size_t i = 0; i < idx2.size(); ++i)
			Res[i] = A(idx1, idx2[i]);

		return Res;
	}

	template <typename Tv>
	inline DynamicVector<Tv> index(const CompressedMatrix<Tv, blaze::columnMajor>& A, const vector<size_t>& idx) {
		return index(A, idx, 0);
	}

	//vector index of vector
	template <typename Tv>
	inline DynamicVector<Tv> index(const DynamicVector<Tv>& vec, const vector<size_t>& idx) {
		DynamicVector<Tv>Res(idx.size());

		for (size_t i = 0; i < idx.size(); i++) {
			Tv v = vec[idx[i]];
			Res[i] = v;
		}

		return Res;
	}

	//Vec[index]=
	template <typename Tv>
	inline void index(DynamicVector<Tv>& vout, const vector<size_t>& idx, const DynamicVector<Tv>& vin) {
		assert(idx.size() == vin.size());

		for (size_t i = 0; i < idx.size(); ++i) {
			vout[idx[i]] = vin[i];
		}
	}

	template <typename Tv>
	void index(CompressedMatrix<Tv, blaze::columnMajor>& mout, const vector<size_t>& idx, size_t idx2, const DynamicVector<Tv>& vin);

	template <typename Tv>
	DynamicVector<Tv>indexbool(const DynamicVector<Tv>& vect, const vector<bool>& idx);

	inline vector<size_t> indexbool(const vector<size_t>& vect, const vector<bool>& idx) {
		assert(vect.size() == idx.size());

		vector<size_t> v;

		for (size_t i = 0; i < idx.size(); i++)
			if (idx[i])
				v.push_back(vect[i]);

		return v;
	}

	template <typename Tv>
	inline bool testZeroDiag(const Tv& a) {

		size_t n = a.rows();
		for (size_t i = 0; i < n; i++) {
			if (abs(a(i, i)) > 0)
				return false;
		}

		return true;
	}

	template <typename Tv>
	inline DynamicVector<Tv>dynvec(const vector<Tv>& v) {
		DynamicVector<Tv>res(v.size());
		for (auto i = 0; i < v.size(); ++i) {
			res[i] = v[i];
		}
		return res;
	}

	const vector<vector<size_t>> vecToComps(vector<size_t>& compvec);

	template<typename Tv>
	tuple<vector<size_t>, vector<size_t>, DynamicVector<Tv>> findnz(const CompressedMatrix<Tv, blaze::columnMajor>& mat);

	// Returns vector where comp[i]=component number
	template<typename Tv>
	vector<size_t> components(const CompressedMatrix<Tv, blaze::columnMajor>& mat);

	template<typename Tv>
	vector<size_t> components(const SparseMatrixCSC<Tv>& mat);

	//Kronecker product
	//for matrices: https://en.wikipedia.org/wiki/Kronecker_product

	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> kron(const CompressedMatrix<Tv,
		blaze::columnMajor>& A, const CompressedMatrix<Tv, blaze::columnMajor>& B);

	// for vectors
	template<typename Tv>
	inline DynamicVector<Tv> kron(const DynamicVector<Tv>& A, const DynamicVector<Tv>& B) {
		DynamicVector<Tv> res(A.size() * B.size());

		for (size_t i = 0; i < A.size(); i++) {
			subvector(res, i * B.size(), B.size()) = A[i] * B;
		}

		return res;
	}

	//Returns the diagonal weighted degree matrix(as a sparse matrix) of a graph

	template<typename Tv>
	inline CompressedMatrix<Tv, blaze::columnMajor> diagmat(const CompressedMatrix<Tv, blaze::columnMajor>& A) {
		return Diagonal(sum(A));
	}

	template<typename Tv>
	inline CompressedMatrix<Tv, blaze::columnMajor> pow(const CompressedMatrix<Tv, blaze::columnMajor>& A, const int n) {
		CompressedMatrix<Tv, blaze::columnMajor> res;
		res = A;
		for (size_t i = 0; i < n - 1; i++) {
			res *= A;
		}
		return res;
	}

	template<typename Tv>
	inline vector<size_t> sortedIndices(const vector<size_t>& I, const vector<size_t>& J, const DynamicVector<Tv>& V) {
		size_t nnz = I.size();
		vector<size_t>idcs = collect(0, nnz);

		sort(idcs.begin(), idcs.end(), [&I, &J, &V](size_t idx1, size_t idx2) {
			return J[idx1] < J[idx2] || J[idx1] == J[idx2] && I[idx1] < I[idx2]; });

		return idcs;
	}

	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> sparse(const vector<size_t>& I, const vector<size_t>& J, const DynamicVector<Tv>& V,
		size_t m, size_t n, bool sort = false);

	template<typename Tv>
	inline CompressedMatrix<Tv, blaze::columnMajor> power(const CompressedMatrix<Tv, blaze::columnMajor>& A, const int k) {
		CompressedMatrix<Tv, blaze::columnMajor>ap;
		ap = pow(A, k);
		ap = ClearDiag(ap);
		return ap;
	}

	// Returns true if graph is connected.
	template<typename Tv>
	inline bool isConnected(const CompressedMatrix<Tv, blaze::columnMajor>& mat) {
		if (!mat.rows())
			return false;

		vector<size_t> cm = components(mat);
		return *max_element(cm.begin(), cm.end()) == 1;
	}

	template<typename Tv>
	pair<Tv, size_t> findmax(const CompressedMatrix<Tv, blaze::columnMajor>& A, int wise = 1);

	template<typename Tv>
	pair<Tv, size_t> findmax(const DynamicVector<Tv>& v);

	template<typename Tv>
	inline Tv mean(const CompressedMatrix<Tv, blaze::columnMajor>& A) {
		return blaze::sum(A) / (A.rows() * A.columns());
	}

	template<typename Tv>
	inline Tv mean(DynamicVector<Tv> v) {
		return blaze::sum(v) / v.size();
	}

	//Returns the upper triangle of M starting from the kth superdiagonal.
	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> triu(const CompressedMatrix<Tv, blaze::columnMajor>& A, size_t k = 0);

	//generates vector with values of [start...end)
	inline vector<size_t> collect(size_t start, size_t end) {
		vector<size_t> res(end);

		for (size_t i = 0; i < end; i++)
			res[i] = i;

		return res;
	}

	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> wtedEdgeVertexMat(const CompressedMatrix<Tv, blaze::columnMajor>& mat);

	inline size_t keyMap(size_t x, size_t n) {
		return x <= n ? x : n + x / n;
	}

	// Create a Laplacian matrix from an adjacency matrix. We might want to do this differently, say by enforcing symmetry

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> lap(CompressedMatrix<Tv, blaze::columnMajor> A);

	//	lap function analog. Create a Laplacian matrix from an adjacency matrix.
	//	If the input looks like a Laplacian, throw a warning and convert it.

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> forceLap(const CompressedMatrix<Tv, blaze::columnMajor>& a);

	template <typename Tv>
	IJV<Tv> path_graph_ijv(size_t n);

	template <typename Tv>
	IJV<Tv> product_graph(IJV<Tv> b, IJV<Tv> a);

	template <typename Tv>
	IJV<Tv> grid2_ijv(size_t n, size_t m, Tv isotropy = 1);

	template <typename Tv>
	inline IJV<Tv> grid2_ijv(size_t n) {
		return grid2_ijv<Tv>(n, n);
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor>grid2(size_t n, size_t m, Tv isotropy = 1);

	template <typename Tv>
	inline CompressedMatrix<Tv, blaze::columnMajor>grid2(size_t n) {
		if (n < 2)
			throw("Parameter n must more than 2");

		return grid2<Tv>(n, n);
	}

	template <typename Tv>
	void matrixToFile(CompressedMatrix<Tv, blaze::columnMajor> A, string FileName);
}

#include "graphalgs.cpp"
