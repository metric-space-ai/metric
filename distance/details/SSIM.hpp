#ifndef _METRIC_DISTANCE_SSIM_HPP
#define _METRIC_DISTANCE_SSIM_HPP
#include <vector>

namespace metric
{
    namespace distance
    {
/*** structural similartiy (for images) ***/
        template <typename V>
        struct SSIM{
            using value_type = V;
            using distance_type = typename V::value_type;


            typename V::value_type dynamic_range = 255.0;
            typename V::value_type masking = 2.0;

            explicit SSIM() = default;
            SSIM(const typename V::value_type dynamic_range_,
                 const typename V::value_type masking_):dynamic_range(dynamic_range_),
                                                        masking(masking_) {}

            template<typename Container>
            distance_type  operator()(const Container &img1, const Container &img2) const;
        };
    }
}

#include "SSIM.cpp"


#endif // Header Guard
