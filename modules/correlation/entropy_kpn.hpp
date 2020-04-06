/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/


#ifndef _ENTROPY_KPN_HPP
#define _ENTROPY_KPN_HPP

#include "../k-related/Standards.hpp"


namespace metric {

// MOVED TO ENTROPY.*PP
/*
template <typename RecType, typename Metric = metric::Chebyshev<typename RecType::value_type>>
struct entropy_kpN {

    template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
    double operator()(
            const OuterContainer<Container, OuterAllocator> X,
            Metric metric = Metric(),
            size_t k = 7,
            size_t p = 70
            ) const;

    template <typename Container>
    double estimate(
            const Container & a,
            const size_t sampleSize = 250,
            const double threshold = 0.05,
            size_t maxIterations = 1000,
            std::size_t k = 3,
            double logbase = 2,
            Metric metric = Metric(),
            bool exp = false
            );
};
*/


} // namespace metric

#include "entropy_kpn.cpp"
#endif
