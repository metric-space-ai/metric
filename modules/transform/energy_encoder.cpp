/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/
#include "energy_encoder.hpp"

#include "wavelet.hpp"

#include <stack>

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
        std::stack<std::size_t> & subband_length,
        int wavelet_type,
        std::size_t subbands_num
        ) {
    OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> out;
    if (x.size()*2 <= subbands_num) {
        for (std::size_t el = 0; el<x.size(); ++el) {
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
        std::stack<std::size_t> & subband_length,
        int wavelet_type
        ) {
    OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> x;
    if (in.size() > 1) {
        for (std::size_t el = 0; el<in.size(); el+=2) { // we assume size of deque is even, TODO check
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
        std::stack<std::size_t> & subband_length,
        int wavelet_type,
        std::size_t subbands_num
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
        std::stack<std::size_t> & subband_length,
        int wavelet_type
        ) {
    OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> outer_out = DWT_unsplit(in, subband_length, wavelet_type);
    return outer_out[0]; // TODO rename deque_out also in DSPCC
}


EnergyEncoder::EnergyEncoder(int wavelet_type, std::size_t splits, bool bothsided)
    : wavelet_type(wavelet_type)
    , bothsided(bothsided) // not used now, TODO implement one-sided!
{
    subbands = 1;
    for (std::size_t i = 0; i < splits; ++i)
        subbands *= 2;

}

template <
    template <typename, typename> class Container,
    typename Allocator,
    typename ValueType
    >
auto EnergyEncoder::operator () (Container<ValueType, Allocator> & in) const -> Container<ValueType, Allocator>
{

    using InnerContainer = Container<ValueType, Allocator>;

    InnerContainer out;

    //subband_length = std::stack<std::size_t>();
    std::stack<std::size_t> subband_length;
    auto n_subbands = sequential_DWT<std::vector, InnerContainer, std::allocator<InnerContainer>>(in, subband_length, wavelet_type, subbands); // TODO update splits with log2

    for (std::size_t i = 0; i<n_subbands.size(); ++i) { // computing one energy value per subband
        ValueType sum = 0;
        for (std::size_t j = 0; j<n_subbands[i].size(); ++j)
            sum += n_subbands[i][j]; // finding mean
        ValueType mean = sum/n_subbands[i].size();
        sum = 0;
        ValueType normalized;
        for (std::size_t j = 0; j<n_subbands[i].size(); ++j) {
            normalized = n_subbands[i][j] - mean;
            //sum += n_subbands[i][j] * n_subbands[i][j];
            sum += normalized * normalized;
        }
        sum = sum / n_subbands[i].size();
        out.push_back(sum);
    }
    return out;
}

auto EnergyEncoder::freq_bounds(std::size_t len) const -> std::vector<std::size_t>
{
    std::vector<std::size_t> bounds = {0};
    float step = len / (2.0 * (float)subbands);
    for (std::size_t i = 1; i<=subbands; ++i)
        bounds.push_back(step*i + 0.5);
    return bounds;
}


// functions for computing sizes

std::size_t subband_size(std::size_t original_size, std::size_t depth, std::size_t wavelet_length) {
    std::size_t sz = original_size;
    for (std::size_t i=1; i<=depth; ++i){
        sz = (sz + wavelet_length - 1)/2.0;
    }
    return sz;
}


std::size_t original_size_old(std::size_t subband_size, std::size_t depth, std::size_t wavelet_length) { // rounding issue
    std::size_t n = 1;
    float sum = 0;
    for (std::size_t i=1; i<=depth; ++i){
        n = n*2;
        sum += (wavelet_length - 2)/(float)n; // -2 instead of -1 because of floor
    }
    return n*(subband_size - sum);
}


std::size_t original_size(std::size_t subband_size, std::size_t depth, std::size_t wavelet_length) {
    std::size_t n = 1;
    std::size_t sz = subband_size;
    for (std::size_t i=1; i<=depth; ++i){
        sz = sz*2 - wavelet_length + 2;
    }
    return sz;
}


std::size_t wmaxlevel(std::size_t input_size, int waveletType) {
    return 0;
}


}  // namespace metric
