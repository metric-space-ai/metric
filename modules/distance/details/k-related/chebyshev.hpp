/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_DISTANCE_DETAILS_K_RELATED_CHEBYSHEV_HPP
#define _METRIC_DISTANCE_DETAILS_K_RELATED_CHEBYSHEV_HPP

#include <cstdlib>
namespace  metric {
    namespace distance {
        template<typename V>
        struct Chebyshev {
            using value_type = V;
            using distance_type = value_type;

            explicit Chebyshev() = default;

            template<typename Container>
            V operator()(const Container & lhs, const Container & rhs) const {
                distance_type res = 0;
                for(std::size_t i = 0; i < lhs.size(); i++) {
                    auto m = std::abs(lhs[i]- rhs[i]);
                    if(m > res)
                        res = m;
                }
                return res;
            }
        };
    }
}
#endif
