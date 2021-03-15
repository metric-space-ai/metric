/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_ENERGY_ENCODER_HPP
#define _METRIC_ENERGY_ENCODER_HPP

#include <cstddef>
#include <vector>

namespace metric {


// functor for computing energy
// very simple and inoptimal recursive functor without static members, to be used as reference
class EnergyEncoder {
public:
    EnergyEncoder(int wavelet_type = 4, std::size_t splits = 2, bool bothsided = true);

    template <
        template <typename, typename> class Container,
        typename Allocator,
        typename ValueType
        >
    Container<ValueType, Allocator> operator () (Container<ValueType, Allocator> & in) const;

    std::vector<std::size_t> freq_bounds(std::size_t len) const;

private:
    int wavelet_type;
    std::size_t subbands;
    bool bothsided;
};


// functions for computing sizes
std::size_t subband_size(std::size_t original_size, std::size_t depth, std::size_t wavelet_length);

std::size_t original_size(std::size_t subband_size, std::size_t depth, std::size_t wavelet_length);

}  // namespace metric

#include "energy_encoder.cpp"

#endif
