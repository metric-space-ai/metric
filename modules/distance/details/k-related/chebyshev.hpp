#ifndef CHEBYSHEV_H_GUARD
#define CHEBYSHEV_H_GUARD
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
