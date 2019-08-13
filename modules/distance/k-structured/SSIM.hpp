#ifndef _METRIC_DISTANCE_DETAILS_K_STRUCTURED_SSIM_HPP
#define _METRIC_DISTANCE_DETAILS_K_STRUCTURED_SSIM_HPP
/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/


namespace metric
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
        
} // namespace distance

#include "SSIM.cpp"


#endif // Header Guard
