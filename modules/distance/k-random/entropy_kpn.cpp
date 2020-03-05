
#ifndef _ENTROPY_KPN_CPP
#define _ENTROPY_KPN_CPP

#define _USE_MATH_DEFINES

#include "epmgp.cpp"
#include "../k-related/Standards.hpp"
#include "../../space/tree.hpp"


namespace metric {



double mvnpdf(blaze::DynamicVector<double> x, blaze::DynamicVector<double> mu, blaze::DynamicMatrix<double> Sigma) {

    size_t n = x.size();
    assert(mu.size() == n && Sigma.columns() == n && Sigma.rows() == n);

    auto centered = x - mu;
    auto p =  blaze::trans(centered) * blaze::inv(Sigma) * centered;
    //return std::exp(-p/2) / ( std::sqrt(blaze::det(Sigma)) * std::pow(2*M_PI, (double)n/2.0) );
    return std::exp(-p/2) / std::sqrt( blaze::det(Sigma) * std::pow(2*M_PI, n) );
}


double mvnpdf(blaze::DynamicVector<double> x) {

    return( mvnpdf(x, blaze::DynamicVector<double>(x.size(), 0), blaze::IdentityMatrix<double>(x.size())) );
}


template <typename recType, typename Metric>
double entropy_kpN(
        std::vector<recType> X,
        Metric metric = metric::Chebyshev<double>(), // temporarily we support only double // TODO update
        size_t k = 7,
        size_t p = 70
        )
{

    size_t n = X.size();
    size_t d = X[0].size();

    double h = 0;

    metric::Tree<recType, Metric> tree (X, -1, metric);
    blaze::DynamicMatrix<double> Nodes (p, d, 0);
    blaze::DynamicVector<double> mu (d, 0);
    blaze::DynamicVector<double> lb (d, 0);
    blaze::DynamicVector<double> ub (d, 0);
    blaze::DynamicVector<double> x_vector (d, 0);
    for (size_t i = 0; i < n; ++i) {

        auto res = tree.knn(X[i], p + 1);
        auto eps = res[k].second;

        blaze::reset(mu);
        for (size_t p_idx= 1; p_idx < p; ++p_idx) { // r v realizations from the tree
            for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
                Nodes(p_idx, d_idx) = res[p_idx].first->data[d_idx];
                mu[d] += res[p_idx].first->data[d_idx];
            }
        }
        auto K = blaze::evaluate( (blaze::trans(Nodes) * Nodes)*p + blaze::IdentityMatrix<double>(d)*double(1e-8) );

        blaze::reset(lb);
        blaze::reset(ub);
        blaze::reset(mu);
        for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
            lb[d_idx] = X[i][d_idx] - eps;
            ub[d_idx] = X[i][d_idx] + eps;
            x_vector[d_idx] = X[i][d_idx];
        }

        auto g_local = epmgp::local_gaussian_axis_aligned_hyperrectangles(mu, K, lb, ub);
        double logG = std::get<0>(g_local);

        double g = mvnpdf(x_vector, mu, K);

        h += logG - std::log(g);

    }

    return h;
}




} // namespace metric


#endif  // _ENTROPY_KPN_CPP
