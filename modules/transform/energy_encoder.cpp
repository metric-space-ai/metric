/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_ENERGY_ENCODER_CPP
#define _METRIC_ENERGY_ENCODER_CPP

//#include <cstddef>
#include <stack>
#include "transform/wavelet.hpp"

namespace metric {



// recursive split for arbitrary depth
// this code is COPIED from DSPCC, TODO update DSPCC to remove code dubbing if this code becomes common


template <
        template <typename, typename> class OuterContainer,
        template <typename, typename> class InnerContainer,
        typename ValueType,
        typename OuterAllocator,
        typename InnerAllocator
        >
OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator>
DWT_split(
        OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> x,
        std::stack<size_t> & subband_length,
        int wavelet_type,
        size_t subbands_num
        ) {
    OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> out;
    if (x.size()*2 <= subbands_num) {
        for (size_t el = 0; el<x.size(); ++el) {
            auto split = wavelet::dwt(x[el], wavelet_type);
            out.push_back(std::get<0>(split));
            out.push_back(std::get<1>(split));
        }
        subband_length.push(x[0].size());
        return DWT_split(out, subband_length, wavelet_type, subbands_num);
    } else {
        return x;
    }
}


template <
        template <typename, typename> class OuterContainer,
        template <typename, typename> class InnerContainer,
        typename ValueType,
        typename OuterAllocator,
        typename InnerAllocator
        >
OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator>
DWT_unsplit(
        OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> in,
        std::stack<size_t> & subband_length,
        int wavelet_type
        ) {
    OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> x;
    if (in.size() > 1) {
        for (size_t el = 0; el<in.size(); el+=2) { // we assume size of deque is even, TODO check
            x.push_back(wavelet::idwt(in[el], in[el+1], wavelet_type, subband_length.top()));
        }
        subband_length.pop();
        return DWT_unsplit(x, subband_length, wavelet_type);
    } else {
        return in;
    }
}




template <
        template <typename, typename> class OuterContainer,
        class InnerContainer,
        typename OuterAllocator
        >
OuterContainer<InnerContainer, OuterAllocator> // TODO better use -> for deduction by return value
sequential_DWT(
        InnerContainer x,
        std::stack<size_t> & subband_length,
        int wavelet_type,
        size_t subbands_num
        ) {
    OuterContainer<InnerContainer, OuterAllocator> outer_x = {x}; // TODO also rename deque_x to outer_x in DSPCC
    return DWT_split(outer_x, subband_length, wavelet_type, subbands_num);
}



template <
        template <typename, typename> class OuterContainer,
        template <typename, typename> class InnerContainer,
        typename ValueType,
        typename OuterAllocator,
        typename InnerAllocator
        >
InnerContainer<ValueType, InnerAllocator>
sequential_iDWT(
        OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> in,
        std::stack<size_t> & subband_length,
        int wavelet_type
        ) {
    OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> outer_out = DWT_unsplit(in, subband_length, wavelet_type);
    return outer_out[0]; // TODO rename deque_out also in DSPCC
}




// functor for computing energy


class energy_encoder { // very simple and inoptimal recursive functor without static members, to be used as reference

public:
    energy_encoder(int wavelet_type_ = 4, size_t splits_ = 4, bool bothsided_ = true) :
        wavelet_type(wavelet_type_),
        splits(splits_),
        bothsided(bothsided_) // not used
    {}

    template <
        template <typename, typename> class Container,
        typename Allocator,
        typename ValueType
        >
    Container<ValueType, Allocator> operator () (Container<ValueType, Allocator> & in) {

        using InnerContainer = Container<ValueType, Allocator>;

        InnerContainer out;

        std::stack<size_t> subband_length;
        auto subbands = sequential_DWT<std::vector, InnerContainer, std::allocator<InnerContainer>>(in, subband_length, wavelet_type, splits); // TODO update splits with log2

        for (size_t i = 0; i<subbands.size(); ++i) { // computing one energy value per subband
            ValueType sum = 0;
            for (size_t j = 0; j<subbands[i].size(); ++j) {
                sum += subbands[i][j] * subbands[i][j];
            }
            sum = sum / subbands[i].size();
            out.push_back(sum);
        }
        return out;
    }

private:
    int wavelet_type;
    size_t splits;
    bool bothsided;

};






}  // namespace metric

#endif
