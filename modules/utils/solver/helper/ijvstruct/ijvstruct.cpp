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

#ifndef _METRIC_UTILS_SOLVER_HELPER_IJVSTRUCT_CPP
#define _METRIC_UTILS_SOLVER_HELPER_IJVSTRUCT_CPP
namespace metric {
template <typename Tv>
IJV<Tv> IJV<Tv>::operator+(const IJV& b) const
{
    IJV<Tv> m;

    m.n = n;
    m.nnz = nnz + b.nnz;
    m.i = i;
    m.j = j;
    m.v = v;

    //Append vectors
    m.i.insert(m.i.end(), b.i.begin(), b.i.end());
    m.j.insert(m.j.end(), b.j.begin(), b.j.end());

    m.v.resize(m.nnz);

    for (size_t i = v.size(); i < m.nnz; i++) {
        m.v[i] = b.v[i - v.size()];
    }

    return m;
}

template <typename Tv>
IJV<Tv>::IJV(const blaze::CompressedMatrix<Tv, blaze::columnMajor>& mat)
{

    n = mat.columns();
    nnz = mat.nonZeros();

    i.resize(nnz);
    j.resize(nnz);
    v.resize(nnz);

    size_t k = 0;

    //Fill i, row and v

    for (size_t l = 0UL; l < mat.columns(); ++l) {

        for (typename blaze::CompressedMatrix<Tv, blaze::columnMajor>::ConstIterator it = mat.cbegin(l); it != mat.cend(l); ++it) {

            i[k] = it->index();
            j[k] = l;
            v[k] = it->value();

            ++k;
        }
    }
}

template <typename Tv>
IJV<Tv>::IJV(const SparseMatrixCSC<Tv>& cscm)
{

    n = cscm.n;
    nnz = cscm.nzval.size();

    i.resize(nnz);
    j.resize(nnz);
    v.resize(nnz);

    size_t k = 0;

    for (size_t l = 0; l != n; l++) {
        size_t colbegin = cscm.colptr[l];
        size_t colend = cscm.colptr[l + 1];

        for (size_t row = colbegin; row != colend; row++) {
            i[k] = cscm.rowval[row];
            j[k] = l;
            v[k] = cscm.nzval[row];
            ++k;
        }
    }
}

template <typename Tv>
void IJV<Tv>::dump_ijv(int ijvn) const
{

    std::cout << "ijv" << ijvn << " matrix dump:\n";

    std::cout << "\n"
              << "n= " << n;
    std::cout << "\n"
              << "nnz= " << nnz;

    std::cout << "\ni=";
    for (size_t k = 0; k < nnz; ++k)
        std::cout << i[k] << " ";

    std::cout << "\n"
              << "j=";
    for (size_t k = 0; k < nnz; ++k)
        std::cout << j[k] << " ";

    std::cout << "\n"
              << "v= ";
    for (size_t k = 0; k < nnz; ++k)
        std::cout << v[k] << " ";

    std::cout << std::endl << std::endl;
}

template <typename Tv>
void IJV<Tv>::sortByCol()
{
    std::vector<size_t> idcs = collect(0, nnz);

    sort(idcs.begin(), idcs.end(),
        [this](size_t idx1, size_t idx2) { return j[idx1] < j[idx2] || (j[idx1] == j[idx2] && i[idx1] < i[idx2]); });

    std::vector<size_t> ni(nnz), nj(nnz);
    blaze::DynamicVector<Tv> nv(nnz);

    for (size_t k = 0; k < nnz; k++) {
        ni[k] = i[idcs[k]];
        nj[k] = j[idcs[k]];
        nv[k] = v[idcs[k]];
    };

    i = std::move(ni);
    j = std::move(nj);
    v = std::move(nv);
}

template <typename Tv>
SparseMatrixCSC<Tv> sparseCSC(const IJV<Tv>& ijv)
{
    SparseMatrixCSC<Tv> res;
    res.m = ijv.n;
    res.n = res.m;

    size_t nnz = ijv.nnz;

    res.colptr.resize(res.n + 1);
    res.rowval.resize(nnz);
    res.nzval.resize(nnz);

    res.colptr[0] = 0;
    res.colptr[res.n] = nnz;

    size_t k = 0;

    //Fill colptr, rowval and nzval

    std::size_t totalnz = 0, t = 0;

    for (size_t l = 0UL; l < res.n; ++l) {
        std::size_t rownz = 0;

        while (t < nnz && l == ijv.j[t]) {

            res.nzval[k] = ijv.v[t];
            res.rowval[k] = ijv.i[t];
            ++k;
            ++rownz;
            ++t;
        }
        totalnz += rownz;
        res.colptr[l + 1] = totalnz;
    }

    return res;
}
}
#endif