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
#include "wavelet.hpp"

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
    //energy_encoder(int wavelet_type_ = 4, size_t splits_ = 4, bool bothsided_ = true) :
    energy_encoder(int wavelet_type_ = 4, size_t splits_ = 2, bool bothsided_ = true) :
        wavelet_type(wavelet_type_),
        //splits(splits_),
        bothsided(bothsided_) // not used now, TODO implement one-sided!
    {
        subbands = 1;
        for (size_t i = 0; i < splits_; ++i)
            subbands *= 2;

    }

    template <
        template <typename, typename> class Container,
        typename Allocator,
        typename ValueType
        >
    Container<ValueType, Allocator> operator () (Container<ValueType, Allocator> & in) {

        using InnerContainer = Container<ValueType, Allocator>;

        InnerContainer out;

        //subband_length = std::stack<size_t>();
        std::stack<size_t> subband_length;
        auto n_subbands = sequential_DWT<std::vector, InnerContainer, std::allocator<InnerContainer>>(in, subband_length, wavelet_type, subbands); // TODO update splits with log2

        for (size_t i = 0; i<n_subbands.size(); ++i) { // computing one energy value per subband
            ValueType sum = 0;
            for (size_t j = 0; j<n_subbands[i].size(); ++j)
                sum += n_subbands[i][j]; // finding mean
            ValueType mean = sum/n_subbands[i].size();
            sum = 0;
            ValueType normalized;
            for (size_t j = 0; j<n_subbands[i].size(); ++j) {
                normalized = n_subbands[i][j] - mean;
                //sum += n_subbands[i][j] * n_subbands[i][j];
                sum += normalized * normalized;
            }
            sum = sum / n_subbands[i].size();
            out.push_back(sum);
        }
        return out;
    }

    std::vector<size_t> freq_bounds(size_t len) {
        std::vector<size_t> bounds = {0};
        float step = len / (2.0 * (float)subbands);
        for (size_t i = 1; i<=subbands; ++i)
            bounds.push_back(step*i + 0.5);
        return bounds;
    }

private:
    int wavelet_type;
    size_t subbands;
    bool bothsided;
    //std::stack<size_t> subband_length;

};





// functions for computing sizes


size_t subband_size(size_t original_size, size_t depth, size_t wavelet_length) {
    size_t sz = original_size;
    for (size_t i=1; i<=depth; ++i){
        sz = (sz + wavelet_length - 1)/2.0;
    }
    return sz;
}


size_t original_size_old(size_t subband_size, size_t depth, size_t wavelet_length) { // rounding issue
    size_t n = 1;
    float sum = 0;
    for (size_t i=1; i<=depth; ++i){
        n = n*2;
        sum += (wavelet_length - 2)/(float)n; // -2 instead of -1 because of floor
    }
    return n*(subband_size - sum);
}


size_t original_size(size_t subband_size, size_t depth, size_t wavelet_length) {
    size_t n = 1;
    size_t sz = subband_size;
    for (size_t i=1; i<=depth; ++i){
        sz = sz*2 - wavelet_length + 2;
    }
    return sz;
}


size_t wmaxlevel(size_t input_size, int waveletType) {

    return 0;
}


}  // namespace metric

#endif
