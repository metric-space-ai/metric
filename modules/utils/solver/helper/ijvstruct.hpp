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

#ifndef _METRIC_UTILS_SOLVER_IJVSTRUCT_HPP
#define _METRIC_UTILS_SOLVER_IJVSTRUCT_HPP
#include <blaze/Math.h>
#include "sparsecsc.hpp"
#include "graphalgs.hpp"

namespace metric {

/**
	 * @class IJV
	 * 
	 * @brief
	 * 
	 */
template <typename Tv>
class IJV {
public:
    /**
		 * @brief Construct a new IJV object
		 * 
		 */
    IJV()
        : n(0)
        , nnz(0)
        , i(0)
        , j(0)
        , v(0)
    {
    }

    /**
		 * @brief Construct a new IJV object
		 * 
		 * @param a 
		 */
    IJV(const IJV& a)
    {
        n = a.n;
        nnz = a.nnz;

        i = a.i;
        j = a.j;
        v = a.v;
    }

    /**
		 * @brief Construct a new IJV object
		 * 
		 * @param an 
		 * @param annz 
		 * @param ai 
		 * @param aj 
		 * @param av 
		 */
    IJV(const size_t an, const size_t annz, const std::vector<size_t>& ai, const std::vector<size_t>& aj,
        const blaze::DynamicVector<Tv>& av)
    {

        n = an;
        nnz = annz;

        i = ai;
        j = aj;
        v = av;
    }

    /**
		 * @brief Construct a new IJV object
		 * 
		 * @param an 
		 * @param annz 
		 * @param ai 
		 * @param aj 
		 * @param av 
		 */
    IJV(const size_t an, const size_t annz, const blaze::DynamicVector<size_t>& ai, const blaze::DynamicVector<size_t>& aj,
        const blaze::DynamicVector<Tv>& av)
    {

        n = an;
        nnz = annz;

        i.insert(i.begin(), ai.begin(), ai.end());
        j.insert(j.begin(), aj.begin(), aj.end());
        v = av;
    }

    /**
		 * @brief Construct a new IJV object
		 * 
		 * @param mat 
		 */
    IJV(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat);

    /**
		 * @brief Construct a new IJV object
		 * 
		 * @param cscm 
		 */
    IJV(const SparseMatrixCSC<Tv>& cscm);

    /**
		 * @brief 
		 * 
		 * @param a 
		 * @return
		 */
    const IJV& operator=(const IJV& a)
    {
        n = a.n;
        nnz = a.nnz;
        i = a.i;
        j = a.j;
        v = a.v;

        return *this;
    }

    /**
		 * @brief 
		 * 
		 * @param b 
		 * @return
		 */
    IJV operator+(const IJV& b) const;

    /**
		 * @brief 
		 * 
		 * @param b 
		 * @return
		 */
    bool operator==(const IJV& b) const
    {
        bool res = n == b.n && nnz == b.nnz && i == b.i && j == b.j && v == b.v;
        return res;
    }

    /**
		 * @brief 
		 * 
		 * @param x 
		 * @return
		 */
    IJV operator*(const Tv x) const
    {
        IJV m;

        m.n = n;
        m.nnz = nnz;
        m.i = i;
        m.j = j;

        m.v = v * x;

        return m;
    }

    /**
		 * @brief Convert to blaze::CompressedMatrix
		 * 
		 * @return
		 */
    blaze::CompressedMatrix<Tv, blaze::columnMajor> toCompressedMatrix() const { return sparse(i, j, v, n, n, true); }

    /**
		 * @brief 
		 * 
		 * @param ijvn 
		 */
    void dump_ijv(int ijvn) const;

    /**
		 * @brief 
		 * 
		 * @return
		 */
    std::vector<size_t> sortedIndices() const { return metric::sortedIndices(i, j, v); }

    /**
		 * @brief 
		 * 
		 */
    void sortByCol();

    std::size_t n;  // dimension
    std::size_t nnz;  // number of nonzero elements
    std::vector<std::size_t> i;  // row
    std::vector<std::size_t> j;  // col
    blaze::DynamicVector<Tv> v;  //nonzero elements
};

/**
	 * @brief 
	 * 
	 * @param x 
	 * @param ijv 
	 * @return
	 */
template <typename Tv>
IJV<Tv> operator*(const Tv x, const IJV<Tv>& ijv)
{

    return ijv * x;
}

/**
	 * @brief 
	 * 
	 * @param a 
	 * @return
	 */
template <typename Tv>
const std::size_t nnz(const IJV<Tv>& a)
{
    return a.nnz;
}

/**
	 * @brief  convert IJV to SparseMatrixCSC
	 * 
	 * @param ijv 
	 * @return
	 */
template <typename Tv>
SparseMatrixCSC<Tv> sparseCSC(const IJV<Tv>& ijv);

/**
	 * @brief 
	 * 
	 * @param ijv 
	 * @return
	 */
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> sparse(const IJV<Tv>& ijv)
{
    return std::move(ijv.toCompressedMatrix());
}

/**
	 * @brief 
	 * 
	 * @param ijv 
	 * @return
	 */
template <typename Tv>
IJV<Tv> compress(const IJV<Tv>& ijv)
{
    return IJV<Tv>(sparseCSC(ijv));
}

/**
	 * @brief 
	 * 
	 * @param ijv 
	 * @return
	 */
template <typename Tv>
IJV<Tv> transpose(const IJV<Tv>& ijv)
{
    return IJV<Tv>(ijv.n, ijv.nnz, ijv.j, ijv.i, ijv.v);
}
}

#include "ijvstruct/ijvstruct.cpp"

#endif
