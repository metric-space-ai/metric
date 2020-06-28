#ifndef PANDA_METRIC_RIEMANNIAN_HPP
#define PANDA_METRIC_RIEMANNIAN_HPP

#include "../../../modules/distance/k-related/Standards.hpp"
#include "../../../modules/utils/type_traits.hpp"
#include "../../utils/wrappers/lapack.hpp"


namespace metric {

class RiemannianDistance {

public:
    RiemannianDistance() = default;

    template<typename T>
    T operator()(blaze::DynamicMatrix<T> A, blaze::DynamicMatrix<T> B)
    {
        blaze::DynamicVector<T> eigenValues;
        sygv(A, B, eigenValues);

        for (T& e : eigenValues) {
            if (e <= std::numeric_limits<T>::epsilon()) {
                e = 1;
            }
        }

        return sqrt(blaze::sum(blaze::pow(blaze::log(eigenValues), 2)));
    }
};


template <typename C, typename Metric>
double RiemannianDatasetDistance(const C& Xc, const C& Yc, Metric m = Metric()) {

    using T = metric::type_traits::underlying_type_t<C>;

    blaze::DynamicMatrix<T> distancesX (Xc.size(), Xc.size(), 0);
    blaze::DynamicMatrix<T> distancesY (Yc.size(), Yc.size(), 0);

    for (size_t i = 0; i<Xc.size(); ++i) {
        for (size_t j = 0; j<Xc.size(); ++j) {
            auto d = m(Xc[i], Xc[j]);
            if (i < j) { // upper triangular area only
                distancesX(i, j) = -d; // Laplacian matrix
                distancesX(i, i) += d;
                distancesX(j, j) += d;
            }
            //distancesX(i, j) = d; // filling blaze distance matrix for dataset Xc
            //distancesX(j, i) = d;
        }
    }
    for (size_t i = 0; i<Yc.size(); ++i) {
        for (size_t j = 0; j<Yc.size(); ++j) {
            auto d = m(Yc[i], Yc[j]);
            if (i < j) { // upper triangular area only
                distancesY(i, j) = d;
                distancesY(i, i) += d;
                distancesY(j, j) += d;
            }
            //distancesY(i, j) = d; // filling blaze distance matrix for dataset Xc
            //distancesY(j, i) = d;
        }
    }
    //std::cout << distancesX << distancesY; // TODO remove

    //auto rd = metric::RiemannianDistance();
    auto rd = RiemannianDistance();
    return rd(distancesX, distancesY); // applying Riemannian to these distance matrices
}


}

#endif  //PANDA_METRIC_RIEMANNIAN_HPP
