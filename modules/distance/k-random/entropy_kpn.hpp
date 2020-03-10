#ifndef _ENTROPY_KPN_HPP
#define _ENTROPY_KPN_HPP

#include "../k-related/Standards.hpp"


namespace metric {


template <typename recType, typename Metric = metric::Chebyshev<typename recType::value_type>>
struct entropy_kpN {

    template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
    double operator()(
            const OuterContainer<Container, OuterAllocator> X,
            Metric metric = Metric(),
            size_t k = 7,
            size_t p = 70
            ) const;
};



} // namespace metric

#include "entropy_kpn.cpp"
#endif
