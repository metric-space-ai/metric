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

#ifndef _METRIC_UTILS_SOLVER_GRAPHGALS_HPP
#define _METRIC_UTILS_SOLVER_GRAPHGALS_HPP

#include <iostream>
#include "../../../../3rdparty/blaze/Math.h"
#include "../../../../3rdparty/blaze/math/Submatrix.h"
#include "../../../../3rdparty/blaze/math/Subvector.h"
#include <vector>
#include <functional>
#include <random>
#include <fstream>
#include <tuple>
#include <utility>
#include "sparsecsc.hpp"

namespace metric {
template <typename T>
class IJV;

/**
	 * @class Random
	 * 
	 * @brief 
	 * 
	 */
template <typename Tv>
class Random {
public:
    /**
		 * @brief Construct a new Random object
		 * 
		 */
    Random()
        : gen(rd())
    {
    }

    /**
		 * @brief 
		 * 
		 * @return
		 */
    Tv rand0_1() { return std::generate_canonical<Tv, 8 * sizeof(Tv)>(gen); }

    /**
		 * @brief 
		 * 
		 * @return
		 */
    Tv randn() { return rand0_1() * 3.4 - 1.7; }

    /**
		 * @brief 
		 * 
		 * @param sz 
		 * @return
		 */
    blaze::DynamicVector<Tv> randv(size_t sz)
    {
        blaze::DynamicVector<Tv> res(sz);

        for (size_t i = 0; i < sz; i++)
            res[i] = rand0_1();

        return res;
    }

private:
    std::random_device rd;
    std::mt19937 gen;
};

/**
	 * @brief clear main diagonal of input matrix
	 * 
	 * @param A input matrix
	 * @return matrix with zeroes on main diagonal
	 */
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> ClearDiag(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @return
	 */
template <typename Tv>
std::vector<size_t> flipIndex(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @param diag_n 
	 * @return
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> diag(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, size_t diag_n = 0);

/**
	 * @brief 
	 * 
	 * @param V 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> Diagonal(const blaze::DynamicVector<Tv>& V);

/**
	 * @brief Conjugate transpose
	 * 
	 * @param A 
	 * @return
	 */
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> adjoint(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A);

/**
	 * @brief 	Create an adjacency matrix and a diagonal vector from an SDDM-matrix.
	 *          That is, from a Laplacian with added diagonal weights
	 * 
	 * @param sddm 
	 * @return
	 */
template <typename Tv>
inline std::pair<blaze::CompressedMatrix<Tv, blaze::columnMajor>, blaze::DynamicVector<Tv>> adj(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& sddm);

/**
	 * @brief Add a new vertex to a with weights to the other vertices corresponding to diagonal surplus weight.
	 * 
	 * @param a 
	 * @param d 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> extendMatrix(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, blaze::DynamicVector<Tv> d);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @param wise 
	 * @return
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> sum(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, int wise = 1);

/**
	 * @brief vector index of matrix
	 * 
	 * @param A 
	 * @param idx1 
	 * @param idx2 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> index(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, const std::vector<size_t>& idx1, const std::vector<size_t>& idx2);

/**
	 * @brief vector index of matrix
	 * 
	 * @param A 
	 * @param idx1 
	 * @param idx2 
	 * @return
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> index(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, const std::vector<size_t>& idx1, const size_t idx2);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @param idx1 
	 * @param idx2 
	 * @return
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> index(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, const size_t idx1, const std::vector<size_t>& idx2);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @param idx 
	 * @return
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> index(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, const std::vector<size_t>& idx);
/**
	 * @brief vector index of vector
	 * 
	 * @param vec 
	 * @param idx 
	 * @return
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> index(const blaze::DynamicVector<Tv>& vec, const std::vector<size_t>& idx);

/**
	 * @brief 
	 * 
	 * @param vout [out[
	 * @param idx 
	 * @param vin 
	 */
template <typename Tv>
inline void index(blaze::DynamicVector<Tv>& vout, const std::vector<size_t>& idx, const blaze::DynamicVector<Tv>& vin);
/**
	 * @brief 
	 * 
	 * @param mout[out]
	 * @param idx 
	 * @param idx2 
	 * @param vin 
	 */
template <typename Tv>
void index(blaze::CompressedMatrix<Tv, blaze::columnMajor>& mout, const std::vector<size_t>& idx, size_t idx2,
    const blaze::DynamicVector<Tv>& vin);

/**
	 * @brief 
	 * 
	 * @param vect 
	 * @param idx 
	 */
template <typename Tv>
blaze::DynamicVector<Tv> indexbool(const blaze::DynamicVector<Tv>& vect, const std::vector<bool>& idx);

/**
	 * @brief 
	 * 
	 * @param vect 
	 * @param idx 
	 * @return
	 */
inline std::vector<size_t> indexbool(const std::vector<size_t>& vect, const std::vector<bool>& idx);

/**
	 * @brief 
	 * 
	 * @param a 
	 * @return
	 */
template <typename Tv>
inline bool testZeroDiag(const Tv& a);
/**
	 * @brief
	 * 
	 * @param v 
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> dynvec(const std::vector<Tv>& v);
/**
	 * @brief 
	 * 
	 * @param compvec[out]
	 * @return
	 */
const std::vector<std::vector<size_t>> vecToComps(std::vector<size_t>& compvec);

/**
	 * @brief 
	 * 
	 * @param mat 
	 * @return
	 */
template <typename Tv>
std::tuple<std::vector<size_t>, std::vector<size_t>, blaze::DynamicVector<Tv>> findnz(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat);

/**
	 * @brief Returns vector where comp[i]=component number
	 * 
	 * @param mat 
	 * @return
	 */
template <typename Tv>
std::vector<size_t> components(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat);

/**
	 * @brief 
	 * 
	 * @param mat 
	 * @return
	 */
template <typename Tv>
std::vector<size_t> components(const SparseMatrixCSC<Tv>& mat);

/**
	 * @brief Kronecker product
	 *        for matrices: https://en.wikipedia.org/wiki/Kronecker_product
	 * 
	 * @param A 
	 * @param B 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> kron(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, const blaze::CompressedMatrix<Tv, blaze::columnMajor>& B);

/**
	 * @brief Kronecker product
	 *        for vectors: https://en.wikipedia.org/wiki/Kronecker_product
	 * 
	 * @param A 
	 * @param B 
	 * @return
	 */
template <typename Tv>
inline blaze::DynamicVector<Tv> kron(const blaze::DynamicVector<Tv>& A, const blaze::DynamicVector<Tv>& B);

/**
	 * @brief Returns the diagonal weighted degree matrix(as a sparse matrix) of a graph
	 * 
	 * @param A 
	 * @return
	 */
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> diagmat(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A);
/**
	 * @brief 
	 * 
	 * @param A 
	 * @param n 
	 * @return
	 */
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> pow(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, const int n);
/**
	 * @brief generates vector with values of [start...end)
	 * 
	 * @param start 
	 * @param end 
	 * @return
	 */
inline std::vector<size_t> collect(size_t start, size_t end);

/**
	 * @brief 
	 * 
	 * @param I 
	 * @param J 
	 * @param V 
	 * @return
	 */
template <typename Tv>
inline std::vector<size_t> sortedIndices(const std::vector<size_t>& I, const std::vector<size_t>& J, const blaze::DynamicVector<Tv>& V);
/**
	 * @brief 
	 * 
	 * @param I 
	 * @param J 
	 * @param V 
	 * @param m 
	 * @param n 
	 * @param sort 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> sparse(
    const std::vector<size_t>& I, const std::vector<size_t>& J, const blaze::DynamicVector<Tv>& V, size_t m, size_t n, bool sort = false);

/**
     * @brief 
     * 
     * @param A 
     * @param k 
     * @return
     */
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> power(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, const int k);

/**
	 * @brief Returns true if graph is connected.
	 * 
	 * @param mat 
	 * @return true if graph is connected.
	 */
template <typename Tv>
inline bool isConnected(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @param wise 
	 * @return
	 */
template <typename Tv>
std::pair<Tv, size_t> findmax(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, int wise = 1);

/**
	 * @brief 
	 * 
	 * @param v 
	 * @return
	 */
template <typename Tv>
std::pair<Tv, size_t> findmax(const blaze::DynamicVector<Tv>& v);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @return
	 */
template <typename Tv>
inline Tv mean(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A);

/**
	 * @brief 
	 * 
	 * @param v 
	 * @return
	 */
template <typename Tv>
inline Tv mean(blaze::DynamicVector<Tv> v);

/**
	 * @brief Returns the upper triangle of M starting from the kth superdiagonal.
	 * 
	 * @param A 
	 * @param k 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> triu(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& A, size_t k = 0);

/**
	 * @brief 
	 * 
	 * @param mat 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> wtedEdgeVertexMat(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat);

/**
	 * @brief 
	 * 
	 * @param x 
	 * @param n 
	 * @return
	 */
inline size_t keyMap(size_t x, size_t n);

/**
	 * @brief Create a Laplacian matrix from an adjacency matrix. We might want to do this differently, say by enforcing symmetry
	 * 
	 * @param A 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> lap(blaze::CompressedMatrix<Tv, blaze::columnMajor> A);

/**
	 * @brief lap function analog. Create a Laplacian matrix from an adjacency matrix.
	 * 		  If the input looks like a Laplacian, throw a warning and convert it.
	 * 
	 * @param a 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> forceLap(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a);

/**
	 * @brief 
	 * 
	 * @param n 
	 * @return
	 */
template <typename Tv>
IJV<Tv> path_graph_ijv(size_t n);

/**
	 * @brief 
	 * 
	 * @param b 
	 * @param a 
	 * @return
	 */
template <typename Tv>
IJV<Tv> product_graph(IJV<Tv> b, IJV<Tv> a);

/**
	 * @brief 
	 * 
	 * @param n 
	 * @param m 
	 * @param isotropy 
	 * @return
	 */
template <typename Tv>
IJV<Tv> grid2_ijv(size_t n, size_t m, Tv isotropy = 1);

/**
	 * @brief 
	 * 
	 * @param n 
	 * @return
	 */
template <typename Tv>
inline IJV<Tv> grid2_ijv(size_t n);
/**
	 * @brief 
	 * 
	 * @param n 
	 * @param m 
	 * @param isotropy 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> grid2(size_t n, size_t m, Tv isotropy = 1);

/**
	 * @brief 
	 * 
	 * @param n 
	 * @return
	 */
template <typename Tv>
inline blaze::CompressedMatrix<Tv, blaze::columnMajor> grid2(size_t n);

/**
	 * @brief 
	 * 
	 * @param A 
	 * @param FileName 
	 */
template <typename Tv>
void matrixToFile(blaze::CompressedMatrix<Tv, blaze::columnMajor> A, std::string FileName);
}

#include "graphalgs/graphalgs.cpp"

#endif
