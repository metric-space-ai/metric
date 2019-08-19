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

#ifndef _METRIC_UTILS_SOLVER_APPROXCOL_HPP
#define _METRIC_UTILS_SOLVER_APPROXCOL_HPP
#include <iostream>
#include <stdint.h>
#include "../../../3rdparty/blaze/Math.h"
#include "../../../3rdparty/blaze/math/Submatrix.h"
#include "../../../3rdparty/blaze/math/Subvector.h"
#include <vector>
#include <chrono>
#include <functional>
#include <type_traits>
#include <utility>
#include "helper/graphalgs.hpp"
#include "helper/solvertypes.hpp"
#include "pcg.hpp"
#include "helper/lapwrappers.hpp"

namespace metric {

/**
 * @brief Laplacian solver by Daniel A.Spielman, 2017
 *  This algorithm is an implementation of an approximate edge - by - edge elimination
 *  algorithm inspired by the Approximate Gaussian Elimination algorithm of Kyng and Sachdeva.
 *  approxchol_lap: the main solver.
 * 
 * @param a 
 * @param pcgIts is an array for returning the number of pcgIterations.  Default is length 0, in which case nothing is returned.
 * @param tol is set to 1e-6 by default,
 * @param maxits defaults to MAX_VAL
 * @param maxtime defaults to MAX_VAL.  It measures seconds.
 * @param verbose defaults to false
 * @param params is additional parameters.
 * 		params = ApproxCholParams(order, output)
			order can be one of
			Deg(by degree, adaptive),
			WDeg(by original wted degree, nonadaptive),
			Given
	
 * @return 
 */
template <typename Tv>
inline SubSolver<Tv> approxchol_lap(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a,
                                    std::vector<size_t>& pcgIts,
                                    float tol = 1e-6,
                                    double maxits = 1000,
                                    double maxtime = HUGE_VAL,
                                    bool verbose = false,
                                    ApproxCholParams params = ApproxCholParams());

/*
	LLp elements are all in the same column.
	row tells us the row, and val is the entry.
	val is set to zero for some edges that we should remove.
	next gives the next in the column.It points to itself to terminate.
	reverse is the index into lles of the other copy of this edge,
	since every edge is stored twice as we do not know the order of elimination in advance.
	*/

/**
 * @brief LLp elements are all in the same column.
 * 
 * @tparam Tv 
 */
template <typename Tv>
class LLp {
public:
    /**
     * @brief Construct a new LLp object
     * 
     * @param Arow 
     * @param Aval 
     * @param Anext 
     * @param Areverse 
     */
    LLp(const size_t Arow, const Tv Aval, const LLp* Anext, const LLp* Areverse)
        : row(Arow)
        , val(Aval)
        , next(Anext)
        , reverse(Areverse)
    {
    }
    /**
     * @brief Construct a new LLp object
     * 
     * @param Arow 
     * @param Aval 
     */
    LLp(const size_t Arow, const Tv Aval)
        : row(Arow)
        , val(Aval)
        , next(this)
        , reverse(this)
    {
    }
    
    /**
     * @brief Construct a new LLp object
     * 
     * @param Arow 
     * @param Aval 
     * @param Anext 
     */
    LLp(const size_t Arow, const Tv Aval, LLp* Anext)
        : row(Arow)
        , val(Aval)
        , next(Anext)
        , reverse(this)
    {
    }

    /**
     * @brief row tells us the row
     * 
     */
    size_t row;

    /**
     * @brief  val is the entry. val is set to zero for some edges that we should remove.
     * 
     */
    Tv val;
    
    /**
     * @brief gives the next in the column.It points to itself to terminate.
     * 
     */
    LLp* next;

    /**
     * @brief  the index into lles of the other copy of this edge,
	 * since every edge is stored twice as we do not know the order of elimination in advance.
     * 
     */
    LLp* reverse;

};

/*

	LLmatp is the data structure used to maintain the matrix during elimination.
	It stores the elements in each column in a singly linked list(only next ptrs)
	Each element is an LLp(linked list pointer).
	The head of each column is pointed to by cols.

	We probably can get rid of degs - as it is only used to store initial degrees.

	*/

/**
 * @class LLmatp
 * 
 * @brief the data structure used to maintain the matrix during elimination.
 *        It stores the elements in each column in a singly linked list(only next ptrs)
 *        Each element is an LLp(linked list pointer).
 *        The head of each column is pointed to by cols.
 *        We probably can get rid of degs - as it is only used to store initial degrees.
 * 
 */
template <typename Tv>
class LLmatp {
public:

    /**
     * @brief Construct a new LLmatp object
     * 
     * @param A 
     */
    explicit LLmatp(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A);
    
    /**
     * @brief Destroy the LLmatp object
     * 
     */
    ~LLmatp()
    {
        for (size_t i = 0; i < llelems.size(); i++)
            delete llelems[i];
    }
    
    size_t n;
    std::vector<size_t> degs;
    std::vector<LLp<Tv>*> cols;
    std::vector<LLp<Tv>*> llelems;

};

/**
 * @brief Print a column in an LLmatp matrix.
 *        This is here for diagnostics.
 *
 * @param llmat 
 * @param i 
 */
template <typename Tv>
void print_ll_col(const LLmatp<Tv>& llmat, const size_t i);

//these are the types we use with a fixed ordering

/**
 * @class LLord
 * 
 * @brief 
 * 
 *
 */
template <typename Tv>
class LLord {
public:
    /**
     * @brief Construct a new LLord object
     * 
     * @param Arow 
     * @param Anext 
     * @param Aval 
     */
    LLord(const size_t Arow, const size_t Anext, const Tv Aval)
        : row(Arow)
        , next(Anext)
        , val(Aval)
    {
    }

    size_t row;
    size_t next;
    Tv val;
};

/**
 * @class LLMatOrd
 * @brief
 * 
 */
template <typename Tv>
class LLMatOrd {
public:
    size_t n;
    std::vector<size_t> cols;
    std::vector<LLord<Tv>> lles;
};

/**
 * @class LLcol
 * @brief
 * 
 */
template <typename Tv>
class LLcol {
public:
    /**
     * @brief Construct a new LLcol object
     * 
     * @param Arow 
     * @param Aptr 
     * @param Aval 
     */
    LLcol(const size_t Arow, const size_t Aptr, const Tv Aval)
        : row(Arow)
        , ptr(Aptr)
        , val(Aval)
    {
    }
    size_t row;
    size_t ptr;
    Tv val;

};

// LDLinv

/*  LDLinv contains the information needed to solve the Laplacian systems.
	  It does it by applying Linv, then Dinv, then Linv (transpose).
	  But, it is specially constructed for this particular solver.
	  It does not explicitly make the matrix triangular.
	  Rather, col[i] is the name of the ith col to be eliminated
	*/

/**
 * @class LDLinv
 * @brief LDLinv contains the information needed to solve the Laplacian systems.
 *        It does it by applying Linv, then Dinv, then Linv (transpose).
 *        But, it is specially constructed for this particular solver.
 *        It does not explicitly make the matrix triangular.
 *   	  Rather, col[i] is the name of the ith col to be eliminated
 */
template <typename Tv>
class LDLinv {
public:
    /**
     * @brief Construct a new LDLinv object
     * 
     * @param A 
     */
    explicit LDLinv(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A)
        : col(A.columns() - 1, 0)
        , colptr(A.columns(), 0)
        , d(A.columns(), 0) /* rowval and fval are empty*/
    {
    }

    /**
     * @brief Construct a new LDLinv object
     * 
     * @param A 
     */
    explicit LDLinv(const LLMatOrd<Tv>& A)
        : col(A.n - 1, 0)
        , colptr(A.n, 0)
        , d(A.n, 0)
    {
    }
    
    /**
     * @brief Construct a new LDLinv object
     * 
     * @param A 
     */
    explicit LDLinv(const LLmatp<Tv>& A)
        : col(A.n - 1, 0)
        , colptr(A.n, 0)
        , d(A.n, 0)
    {
    }

    /**
     * @brief 
     * 
     * @param b 
     * @return
     */
    blaze::DynamicVector<Tv> LDLsolver(const blaze::DynamicVector<Tv>& b) const;

    /**
     * @brief 
     * 
     */
    void debug() const;

    std::vector<size_t> col;
    std::vector<size_t> colptr;
    std::vector<size_t> rowval;
    std::vector<Tv> fval;
    std::vector<Tv> d;

private:
    void forward(blaze::DynamicVector<Tv>& y) const;
    void backward(blaze::DynamicVector<Tv>& y) const;

};

/**
 * @class ApproxCholPQElem
 * @brief the data structure we use to keep track of degrees
 * 
 */
class ApproxCholPQElem {
public:
    /**
     * @brief Construct a new ApproxCholPQElem object
     * 
     */
    ApproxCholPQElem() {};

    /**
     * @brief Construct a new ApproxCholPQElem object
     * 
     * @param Aprev 
     * @param Anext 
     * @param Akey 
     */
    ApproxCholPQElem(const size_t Aprev, const size_t Anext, const size_t Akey)
        : prev(Aprev)
        , next(Anext)
        , key(Akey)
    {
    }

    size_t prev = 0;
    size_t next = 0;
    size_t key = 0;
};

/**
 * @class ApproxCholPQ
 * @brief An approximate priority queue.
 *   	  Items are bundled together into doubly-linked lists with all approximately the same key.
 *  	  minlist is the min list we know to be non-empty.
 *  	  It should always be a lower bound.
 *  	  keyMap maps keys to lists
 * 
 */
class ApproxCholPQ {
public:
    /**
     * @brief Construct a new ApproxCholPQ object
     * 
     * @param a 
     */
    explicit ApproxCholPQ(const std::vector<size_t>& a);
    
    /**
     * @brief 
     * 
     * @param i 
     * @param newkey 
     * @param oldlist 
     * @param newlist 
     */
    void move(const size_t i, const size_t newkey, const size_t oldlist, const size_t newlist);
    
    /**
     * @brief 
     * 
     * @param i 
     */
    void inc(const size_t i);

    /**
     * @brief 
     * 
     * @param i 
     */
    void dec(const size_t i);

    /**
     * @brief 
     * 
     * @return
     */
    size_t pop();
    
    /**
     * @brief 
     * 
     */
    void debug();

    std::vector<ApproxCholPQElem> elems;  // indexed by node name
    std::vector<size_t> lists;
    size_t minlist;
    size_t nitems;
    size_t n;

};

//The approximate factorization

/**
 * @brief 
 * 
 * @param llmat 
 * @param i 
 * @param colspace 
 * @return
 */
template <typename Tv>
size_t get_ll_col(const LLmatp<Tv>& llmat, size_t i, std::vector<LLp<Tv>*>& colspace);

/**
 * @brief 
 * 
 * @param colspace 
 * @param len 
 */
template <typename Tv>
void debugLLp(const std::vector<LLp<Tv>*>& colspace, size_t len);

/**
 * @brief 
 * 
 * @param colspace 
 * @param len 
 * @param pq 
 * @return
 */
template <typename Tv>
size_t compressCol(std::vector<LLp<Tv>*>& colspace, size_t len, ApproxCholPQ& pq);

/**
 * @brief 
 * 
 * @param colspace 
 * @param len 
 * @return
 */
template <typename Tv>
size_t compressCol(std::vector<LLcol<Tv>>& colspace, size_t len);

// this one is greedy on the degree - also a big win

/**
 * @brief 
 * 
 * @tparam Tv 
 * @param a 
 * @return
 */
template <typename Tv>
LDLinv<Tv> approxChol(LLmatp<Tv>& a);

/**
 * @brief 
 * 
 * @param a 
 * @param pcgIts 
 * @param tol 
 * @param maxits 
 * @param maxtime 
 * @param verbose 
 * @param params 
 * @return
 */
template <typename Tv>
SubSolver<Tv> approxchol_lapGreedy(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a,
                                   std::vector<size_t>& pcgIts,
                                   float tol = 1e-6,
                                   double maxits = 1000,
                                   double maxtime = HUGE_VAL,
                                   bool verbose = false,
                                   ApproxCholParams params = ApproxCholParams());

/**
 * @brief 
 * 
 * @param a 
 * @param pcgIts 
 * @param tol 
 * @param maxits 
 * @param maxtime 
 * @param verbose 
 * @param params 
 * @return
 */
template <typename Tv>
inline SubSolver<Tv> approxchol_lap1(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a,
                                     std::vector<size_t>& pcgIts,
                                     float tol = 1e-6,
                                     double maxits = 1000,
                                     double maxtime = HUGE_VAL,
                                     bool verbose = false,
                                     ApproxCholParams params = ApproxCholParams())
{
    return approxchol_lapGreedy(a, pcgIts, tol, maxits, maxtime, verbose, params);
}

/**
 * @brief 
 * 
 * @param a 
 * @param pcgIts 
 * @param tol 
 * @param maxits 
 * @param maxtime 
 * @param verbose 
 * @param params 
 * @return SubSolver<Tv> 
 */
template <typename Tv>
inline SubSolver<Tv> approxchol_lap(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a,
                                    std::vector<size_t>& pcgIts,
                                    float tol,
                                    double maxits,
                                    double maxtime,
                                    bool verbose,
                                    ApproxCholParams params)
{
    return lapWrapComponents(SolverA<Tv>(approxchol_lap1<Tv>), a, pcgIts, tol, maxits, maxtime, false);
}

/**
 * @class SqLinOp
 * @brief 
 * 
 */
template <typename Tv>
class SqLinOp {
public:
    /**
     * @brief Construct a new SqLinOp object
     * 
     * @param issym 
     * @param value 
     * @param n 
     * @param multFn 
     */
    SqLinOp(bool issym, Tv value, size_t n, SubSolver<Tv> multFn)
        : issym(issym)
        , value(value)
        , n(n)
        , multFn(multFn) {};

    bool issym;
    Tv value;
    size_t n;
    SubSolver<Tv> multFn;

};

/**
 * @brief 
 * 
 * @param A 
 * @return
 */
template <typename Tv>
std::pair<size_t, size_t> size(const SqLinOp<Tv>& A)
{
    return std::make_pair(A.n, A.n);
}

/**
 * @brief 
 * 
 * @param A 
 * @param d 
 * @return
 */
template <typename Tv>
inline size_t size(const SqLinOp<Tv>& A, size_t d)
{
    return A.n;
}

/**
 * @brief 
 * 
 * @param A 
 * @return true 
 * @return false 
 */
template <typename Tv>
inline bool issymetric(const SqLinOp<Tv>& A)
{
    return A.issym;
}

/**
 * @brief 
 * 
 * @param A 
 * @param b 
 * @return
 */
template <typename Tv>
inline blaze::DynamicVector<Tv> operator*(const SqLinOp<Tv>& A, const blaze::DynamicVector<Tv>& b)
{
    return A.multFn(b);
}

/**
 * @brief 
 * 
 * @param A 
 * @return
 */
template <typename Tv>
size_t checksquare(const SqLinOp<Tv>& A)
{
    return A.n;
}

/**
 * @brief 
 * 
 * @param Y 
 * @param A 
 * @param B 
 */
template <typename Tv>
void mul(blaze::DynamicVector<Tv>& Y, const SqLinOp<Tv>& A, const blaze::DynamicVector<Tv>& B)
{
    blaze::DynamicVector<Tv> Y1 = A * B;

    for (size_t i = 0; i < A.n; i++) {
        Y[i] = Y1[i];
    }
}

// template <typename Tv>
// auto aupd_wrapper(function<blaze::DynamicVector<Tv>(blaze::DynamicVector <Tv>)> matvecA,
//                   function<blaze::DynamicVector<Tv>(blaze::DynamicVector <Tv>)> matvecB,
//                   function<blaze::DynamicVector<Tv>(blaze::DynamicVector <Tv>)> solveSI,
//                   size_t n, bool sym, size_t nev, size_t ncv,
//                   float tol, size_t maxiter, size_t mode, blaze::DynamicVector<Tv> v0) {

// 	size_t lworkl = sym ? ncv * (ncv + 8) : ncv * (3 * ncv + 6);

// 	DynamicMatrix<Tv> v(n, ncv);
// 	blaze::DynamicVector<Tv> workd(3 * n);
// 	blaze::DynamicVector<Tv> workl(lworkl);
// 	blaze::DynamicVector<Tv> rwork;

// 	blaze::DynamicVector<Tv> resid;
// 	size_t info;

// 	if (v0.size() == 0) {
// 		resid.resize(n);
// 		info = 0;
// 	}
// 	else {
// 		resid = v0;
// 		info = 1;
// 	}

// 	std::vector<size_t> iparam(11);
// 	int sz = sym ? 11 : 14;
// 	std::vector<size_t>ipntr(sz);
// 	size_t ido = 0;

// 	iparam[1] = 1; //ishifts
// 	iparam[2] = maxiter; //maxiter
// 	iparam[6] = mode; //mode

// 	zernm1 = collection(0, n);

// 	while (true) {
// 		if (sym)
// 			saupd(ido, n, nev, &tol, resid, ncv, v, n, iparam, ipntr, workd, workl, lworkl, info);

// 	}
// }

// template <typename Tv>
// blaze::DynamicVector<Tv> eigs(SqLinOp<Tv> A, size_t nev = 6, float tol = 0.0F) {
// 	size_t tmp = 2 * nev + 1;
// 	int sigma = -1;
// 	size_t ncv = 20 > tmp ? 20 : tmp;
// 	size_t maxiter = 300;
// 	std::vector<Tv> v0;
// 	bool ritzvec = true;
// 	bool isgeneral = false;
// 	size_t n = checksquare(A);
// 	bool sym = issymmetric(A);
// 	size_t nevmax = sym ? n - 1 : n - 2;

// 	if (nevmax <= 0)
// 		throw("Input matrix A is too small. Use eigen instead.");

// 	if (nev > nevmax)
// 		nev = nevmax;

// 	if (nev == 0)
// 		throw("Requested number of eigenvalues (nev) must be ≥ 1");

// 	ncvmin = nev + (sym ? 1 : 2);

// 	if (ncv < ncvmin)
// 		ncv = ncvmin;

// 	ncv = ncv < nmin ? ncv : n;
// 	bool isshift = sigma > -1;
// 	sigma = isshift ? sigma : 0;

// 	if (v0.size() > 0)
// 		if (v0.size() != n)
// 			throw("Dimension does not mismatch");

// 	function<blaze::DynamicVector<Tv>(blaze::DynamicVector <Tv>)> matvecA, matvecB, solverSI;
// 	matvecA = [&A](blaze::DynamicVector<Tv>& y, const blaze::DynamicVector<Tv>& x) {return mul(y, A, x); };

// 	size_t mode;

// 	if (!isgeneral) {
// 		matvecB = [](x) {return x; };

// 		if (!isshift) {
// 			mode = 1;
// 			solverSI = [](x) {return x; };
// 		}
// 		else {
// 			mode = 3;
// 			//F = factorize(A - UniformScaling(sigma));
// 			//solveSI = [](x) {return F \ x; };
// 		}
// 	}

// 	blaze::DynamicVector<size_t> output;

// 	return output;
// }


}

#include "approxchol/approxchol.cpp"
#endif
