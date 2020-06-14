#ifndef PANDA_METRIC_RIEMANNIAN_HPP
#define PANDA_METRIC_RIEMANNIAN_HPP


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
}

#endif  //PANDA_METRIC_RIEMANNIAN_HPP
