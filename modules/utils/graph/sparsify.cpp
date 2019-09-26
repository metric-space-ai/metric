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
#ifndef _METRIC_UTILS_GRAPH_SPARSIFY_CPP
#define _METRIC_UTILS_GRAPH_SPARSIFY_CPP

namespace metric {
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> sparsify(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, float ep, float matrixConcConst, float JLfac)
{

    Random<double> rnd;
    std::vector<size_t> pcgIts;

    SolverB<Tv> f = approxchol_lap(a, pcgIts, 1e-2F);

    size_t n = a.rows();
    size_t k = (size_t)round(JLfac * std::log(n));  //number of dims for JL

    blaze::CompressedMatrix<Tv, blaze::columnMajor> U = wtedEdgeVertexMat(a);

    size_t m = U.rows();

    blaze::DynamicMatrix<double, blaze::columnMajor> R(m, k);

    for (size_t i = 0; i < m; i++) {

        for (size_t j = 0; j < k; j++)
            R(i, j) = rnd.randn();
    }
    blaze::CompressedMatrix<Tv, blaze::columnMajor> UR = adjoint(U) * R;

    blaze::CompressedMatrix<Tv, blaze::columnMajor> V(n, k, 0);

    for (size_t i = 0; i < k; i++) {
        blaze::DynamicVector<Tv> x, b;

        std::vector<size_t> idx = collect(0, UR.rows());
        b = index(UR, idx, i);

        x = f(b);

        idx = collect(0, V.rows());
        index(V, idx, i, x);
    }

    auto [ai, aj, av] = findnz(triu(a));

    blaze::DynamicVector<Tv> prs(av.size());

    for (size_t h = 0; h < av.size(); h++) {
        size_t i = ai[h];
        size_t j = aj[h];

        blaze::DynamicVector<Tv> vi, vj, vr;

        std::vector<size_t> idx = collect(0, V.columns());
        vi = index(V, i, idx);
        vj = index(V, j, idx);
        Tv nr = std::pow(norm(vi - vj), 2) / k;
        Tv tmp = av[h] * nr * matrixConcConst * std::log(n) / std::pow(ep, 2);
        prs[h] = (1 < tmp) ? 1 : tmp;
    }

    std::vector<bool> ind(prs.size());
    blaze::DynamicVector<double> rndvec = rnd.randv(prs.size());

    for (size_t i = 0; i < prs.size(); i++)
        ind[i] = rndvec[i] < prs[i];

    std::vector<size_t> ai_ind = indexbool(ai, ind);
    std::vector<size_t> aj_ind = indexbool(aj, ind);
    blaze::DynamicVector<Tv> av_ind = indexbool(av, ind);
    blaze::DynamicVector<Tv> prs_ind = indexbool(prs, ind);

    blaze::DynamicVector<Tv> divs = av_ind / prs_ind;

    blaze::CompressedMatrix<Tv, blaze::columnMajor> as = sparse(ai_ind, aj_ind, divs, n, n);

    as = as + adjoint(as);

    return as;
}
}

#endif