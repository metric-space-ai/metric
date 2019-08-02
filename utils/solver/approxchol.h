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


//Structures for the approxChol solver

#pragma once
#include <iostream>
#include <stdint.h>
#include <blaze/Math.h>
#include <blaze/math/Submatrix.h>
#include <blaze/math/Subvector.h>
#include <vector>
#include <chrono>
#include <functional>
#include <type_traits>
#include "graphalgs.h"
#include "solvertypes.h"
#include "pcg_1.h"
#include "lapwrappers.h"

using namespace std;
using blaze::CompressedMatrix;
using blaze::DynamicMatrix;
using blaze::DynamicVector;
using blaze::columnwise;
using blaze::rowwise;

namespace laplacians {

	/*
	LLp elements are all in the same column.
	row tells us the row, and val is the entry.
	val is set to zero for some edges that we should remove.
	next gives the next in the column.It points to itself to terminate.
	reverse is the index into lles of the other copy of this edge,
	since every edge is stored twice as we do not know the order of elimination in advance.
	*/

	template<typename Tv>
	class LLp {
	public:
		size_t row;
		Tv val;

		LLp* next;
		LLp* reverse;

		/*LLp() {
			row = 0;
			val = 0;
			next = this;
			reverse = this;
		}*/

		LLp(const size_t Arow, const Tv Aval, const LLp* Anext, const LLp* Areverse) :row(Arow), val(Aval), next(Anext), reverse(Areverse) {}

		LLp(const size_t Arow, const Tv Aval) :row(Arow), val(Aval), next(this), reverse(this) {}

		LLp(const size_t Arow, const Tv Aval, LLp* Anext) :row(Arow), val(Aval), next(Anext), reverse(this) {
		}
	};

	/*

	LLmatp is the data structure used to maintain the matrix during elimination.
	It stores the elements in each column in a singly linked list(only next ptrs)
	Each element is an LLp(linked list pointer).
	The head of each column is pointed to by cols.

	We probably can get rid of degs - as it is only used to store initial degrees.

	*/

	template <typename Tv>
	class LLmatp {
	public:
		size_t n;
		vector<size_t> degs;
		vector<LLp<Tv>*> cols;
		vector<LLp<Tv>*> llelems;

		LLmatp(const CompressedMatrix<Tv, blaze::columnMajor>& A);
		~LLmatp() {
			for (size_t i = 0; i < llelems.size(); i++)	delete llelems[i];
		}
	};

	//Print a column in an LLmatp matrix.
	//This is here for diagnostics.

	template <typename Tv>
	void print_ll_col(const LLmatp<Tv>& llmat, const size_t i);

	//these are the types we use with a fixed ordering

	template<typename Tv>
	class LLord {
	public:
		size_t row;
		size_t next;
		Tv val;

		LLord(const size_t Arow, const size_t Anext, const Tv Aval) :row(Arow), next(Anext), val(Aval) {}
	};

	template<typename Tv>
	class LLMatOrd {
	public:
		size_t n;
		vector<size_t> cols;
		vector<LLord<Tv>> lles;
	};

	template<typename Tv>
	class LLcol {
	public:
		size_t row;
		size_t ptr;
		Tv val;

		LLcol(const size_t Arow, const size_t Aptr, const Tv Aval) :row(Arow), ptr(Aptr), val(Aval) {}
	};

	// LDLinv

	/*  LDLinv contains the information needed to solve the Laplacian systems.
	  It does it by applying Linv, then Dinv, then Linv (transpose).
	  But, it is specially constructed for this particular solver.
	  It does not explicitly make the matrix triangular.
	  Rather, col[i] is the name of the ith col to be eliminated
	*/

	template<typename Tv>
	class LDLinv {
		void forward(DynamicVector<Tv>& y) const;
		void backward(DynamicVector<Tv>& y) const;
	public:
		vector<size_t> col;
		vector<size_t> colptr;
		vector<size_t> rowval;
		vector<Tv> fval;
		vector<Tv> d;

		LDLinv(const CompressedMatrix<Tv, blaze::columnMajor> &A) : col(A.columns() - 1, 0), colptr(A.columns(), 0), 
			d(A.columns(), 0) /* rowval and fval are empty*/	{}

		LDLinv(const LLMatOrd<Tv> &A) : col(A.n - 1, 0), colptr(A.n, 0), d(A.n, 0) {}

		LDLinv(const LLmatp<Tv> &A) : col(A.n - 1, 0), colptr(A.n, 0), d(A.n, 0) {}
		DynamicVector<Tv> LDLsolver(const DynamicVector<Tv>& b) const;

		void debug() const;
	};

	

	/*
	ApproxCholPQ

	the data structure we use to keep track of degrees
	*/

	class ApproxCholPQElem {
	public:
		size_t prev=0;
		size_t next=0;
		size_t key=0;

		ApproxCholPQElem() {};
		ApproxCholPQElem(const size_t Aprev, const size_t Anext, const size_t Akey) :prev(Aprev), next(Anext), key(Akey) {}
	};

	/*
	  An approximate priority queue.
	  Items are bundled together into doubly-linked lists with all approximately the same key.
	  minlist is the min list we know to be non-empty.
	  It should always be a lower bound.
	  keyMap maps keys to lists
	*/

	class ApproxCholPQ {
	public:
		vector<ApproxCholPQElem> elems; // indexed by node name
		vector<size_t> lists;
		size_t minlist;
		size_t nitems; 
		size_t n;

		ApproxCholPQ(const vector<size_t>& a);

		void move(const size_t i, const size_t newkey, const size_t oldlist, const size_t newlist);
		void inc(const size_t i);
		void dec(const size_t i);
		size_t pop();

		void debug();
	};

	//The approximate factorization

	template<typename Tv>
	size_t get_ll_col(const LLmatp<Tv>& llmat, size_t i, vector<LLp<Tv>*>& colspace);

	template<typename Tv>
	void debugLLp(const vector<LLp<Tv>*>& colspace, size_t len);

	template<typename Tv>
	size_t compressCol(vector<LLp<Tv>*>& colspace, size_t len, ApproxCholPQ& pq);

	template<typename Tv>
	size_t compressCol(vector<LLcol<Tv>>& colspace, size_t len);

	// this one is greedy on the degree - also a big win

	template<typename Tv>
	LDLinv<Tv> approxChol(LLmatp<Tv>& a);

	template<typename Tv>
	SubSolver<Tv> approxchol_lapGreedy(const CompressedMatrix<Tv, blaze::columnMajor>& a, vector<size_t>& pcgIts, float tol = 1e-6,
		double maxits = 1000, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams());

	template<typename Tv>
	inline SubSolver<Tv> approxchol_lap1(const CompressedMatrix<Tv, blaze::columnMajor> &a, vector<size_t>& pcgIts, float tol = 1e-6,
		double maxits = 1000, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams())
	{
		return approxchol_lapGreedy(a, pcgIts, tol, maxits, maxtime, verbose, params);
	}

	template<typename Tv>
	inline SubSolver<Tv> approxchol_lap(const CompressedMatrix<Tv, blaze::columnMajor> &a, vector<size_t>& pcgIts, float tol = 1e-6,
		double maxits = 1000, double maxtime = HUGE_VAL, bool verbose = false,
		ApproxCholParams params = ApproxCholParams())
	{
		return lapWrapComponents(SolverA<Tv>(approxchol_lap1<Tv>), a, pcgIts, tol, maxits, maxtime, false);
	}

	template <typename Tv>
	class SqLinOp {
	public:
		bool issym;
		Tv value;
		size_t n;
		SubSolver<Tv> multFn;

		SqLinOp(bool issym, Tv value, size_t n, SubSolver<Tv> multFn) :issym(issym), value(value), n(n), multFn(multFn) {};
	};

	template <typename Tv>
	pair<size_t, size_t> size(const SqLinOp<Tv>& A) {
		return make_pair(A.n, A.n);
	}

	template <typename Tv>
	inline size_t size(const SqLinOp<Tv>& A, size_t d) {
		return A.n;
	}

	template <typename Tv>
	inline bool issymetric(const SqLinOp<Tv>& A) {
		return A.issym;
	}

	template <typename Tv>
	inline DynamicVector<Tv> operator*(const SqLinOp<Tv>& A, const DynamicVector<Tv> &b) {
		return A.multFn(b);
	}

	
	template <typename Tv>
	size_t checksquare(const SqLinOp<Tv>& A) {
		return A.n;
	}

	template <typename Tv>
	void mul(DynamicVector<Tv>& Y, const SqLinOp<Tv>& A, const DynamicVector<Tv>& B);
	
	template <typename Tv>
	auto aupd_wrapper(function<DynamicVector<Tv>(DynamicVector <Tv>)> matvecA, function<DynamicVector<Tv>(DynamicVector <Tv>)> matvecB,
		function<DynamicVector<Tv>(DynamicVector <Tv>)> solveSI, size_t n, bool sym, size_t nev, size_t ncv,
		float tol, size_t maxiter, size_t mode, DynamicVector<Tv> v0) {

		size_t lworkl = sym ? ncv * (ncv + 8) : ncv * (3 * ncv + 6);

		DynamicMatrix<Tv> v(n, ncv);
		DynamicVector<Tv> workd(3 * n);
		DynamicVector<Tv> workl(lworkl);
		DynamicVector<Tv> rwork;

		DynamicVector<Tv> resid;
		size_t info;

		if (v0.size() == 0) {
			resid.resize(n);
			info = 0;
		}
		else {
			resid = v0;
			info = 1;
		}

		vector<size_t> iparam(11);
		int sz = sym ? 11 : 14;
		vector<size_t>ipntr(sz);
		size_t ido = 0;

		iparam[1] = 1; //ishifts
		iparam[2] = maxiter; //maxiter
		iparam[6] = mode; //mode

		zernm1 = collection(0, n);

		while (true) {
			if (sym)
				saupd(ido, n, nev, &tol, resid, ncv, v, n, iparam, ipntr, workd, workl, lworkl, info);


		}
	}

	template <typename Tv>
	DynamicVector<Tv> eigs(SqLinOp<Tv> A, size_t nev = 6, float tol = 0.0F) {
		size_t tmp = 2 * nev + 1;
		int sigma = -1;
		size_t ncv = 20 > tmp ? 20 : tmp;
		size_t maxiter = 300;
		vector<Tv> v0;
		bool ritzvec = true;
		bool isgeneral = false;
		size_t n = checksquare(A);
		bool sym = issymmetric(A);
		size_t nevmax = sym ? n - 1 : n - 2;

		if (nevmax <= 0)
			throw("Input matrix A is too small. Use eigen instead.");

		if (nev > nevmax)
			nev = nevmax;

		if (nev == 0)
			throw("Requested number of eigenvalues (nev) must be ≥ 1");

		ncvmin = nev + (sym ? 1 : 2);

		if (ncv < ncvmin)
			ncv = ncvmin;

		ncv = ncv < nmin ? ncv : n;
		bool isshift = sigma > -1;
		sigma = isshift ? sigma : 0;

		if (v0.size() > 0)
			if (v0.size() != n)
				throw("Dimension does not mismatch");

		function<DynamicVector<Tv>(DynamicVector <Tv>)> matvecA, matvecB, solverSI;
		matvecA = [&A](DynamicVector<Tv>& y, const DynamicVector<Tv>& x) {return mul(y, A, x); };

		size_t mode;

		if (!isgeneral) {
			matvecB = [](x) {return x; };

			if (!isshift) {
				mode = 1;
				solverSI = [](x) {return x; };
			}
			else {
				mode = 3;
				//F = factorize(A - UniformScaling(sigma));
				//solveSI = [](x) {return F \ x; };
			}
		}

		DynamicVector<size_t> output;

		return output;
	}

	template <typename Tv>
	void mul(DynamicVector<Tv> &Y, const SqLinOp<Tv> &A, const DynamicVector<Tv> &B) {
		DynamicVector<Tv> Y1 = A * B;

		for (size_t i = 0; i < A.n; i++)
		{
			Y[i] = Y1[i];
		}
	}

}


#include "approxchol.cpp"