
#include "DSPCC.hpp"

#include "../../modules/transform/wavelet.hpp"
#include "../../modules/transform/discrete_cosine.hpp"



namespace metric {




template <typename T>
std::deque<std::vector<T>>
sequential_DWT(std::vector<T> x) // TODO add parameters, replace hardcoded depth by recursion
{
    auto [x1, x2] = wavelet::dwt(x, 5);
    auto [x11, x12] = wavelet::dwt(x1, 5);
    auto [x21, x22] = wavelet::dwt(x2, 5);
    auto [x111, x112] = wavelet::dwt(x11, 5);
    auto [x121, x122] = wavelet::dwt(x12, 5);
    auto [x211, x212] = wavelet::dwt(x21, 5);
    auto [x221, x222] = wavelet::dwt(x22, 5);
    std::deque<std::vector<T>> subbands = {x111, x112, x121, x122, x211, x212, x221, x222};
    return subbands;
}


template <typename T>
std::vector<T> sequential_iDWT(std::deque<std::vector<T>> in)
{
    std::vector<T> x;
    size_t len = 0;
    if (in.size() > 0) {
        len = in[0].size();
        auto x22 = wavelet::idwt(in[6], in[7], 5, len);
        auto x21 = wavelet::idwt(in[4], in[5], 5, len);
        auto x12 = wavelet::idwt(in[2], in[3], 5, len);
        auto x11 = wavelet::idwt(in[0], in[1], 5, len);
        auto x2 = wavelet::idwt(x21, x22, 5, len);
        auto x1 = wavelet::idwt(x11, x12, 5, len);
        x = wavelet::idwt(x1, x2, 5, len);
    }
    return x;

}



// recursive split for arbitrary depth
// TODO consider creating special class for DWT split in tree order (with stack encaplulated)

template <typename T>
std::deque<std::vector<T>> DWT_split(
        std::deque<std::vector<T>> x,
        std::stack<size_t> & subband_length,
        int wavelet_type,
        size_t subbands_num
        ) {
    std::deque<std::vector<T>> out;
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


template <typename T>
std::deque<std::vector<T>> DWT_unsplit(
        std::deque<std::vector<T>> in,
        std::stack<size_t> & subband_length,
        int wavelet_type
        ) { // TODO pass stack of sizes!!
    std::deque<std::vector<T>> x;
    if (in.size() > 1) {
        for (size_t el = 0; el<in.size(); el+=2) { // we assume size of deque is even, TODO check
            x.push_back(wavelet::idwt(in[el], in[el+1], wavelet_type, subband_length.top())); // FIXME size value is wrong!! TODO use from stack of sizes
        }
        subband_length.pop();
        return DWT_unsplit(x, subband_length, wavelet_type);
    } else {
        return in;
    }
}


template <typename T>
std::deque<std::vector<T>>
sequential_DWT(
        std::vector<T> x,
        std::stack<size_t> & subband_length,
        int wavelet_type,
        size_t subbands_num
        ) {
    std::deque<std::vector<T>> deque_x = {x};
    return DWT_split(deque_x, subband_length, wavelet_type, subbands_num);
}


template <typename T>
std::vector<T>
sequential_iDWT(
        std::deque<std::vector<T>> in,
        std::stack<size_t> & subband_length,
        int wavelet_type
        ) {
    std::deque<std::vector<T>> deque_out = DWT_unsplit(in, subband_length, wavelet_type);
    return deque_out[0];
}





template <typename recType, typename Metric>
DSPCC<recType, Metric>::DSPCC(
        const std::vector<recType> & TrainingDataset,
        size_t n_features_,
        size_t n_subbands_,
        float time_freq_balance_,
        float DCT_cutoff_
        ) {

    time_freq_balance = time_freq_balance_;
    mix_idx = 0;
    for (size_t n = 4; n<=n_subbands_; n = n*2)
        n_subbands = n;


    // computing 2^n value nearest to given time-freq mix factor
    //mix_idx = mix_index(TrainingDataset[0].size(), time_freq_balance_);

//    size_t rec_length = TrainingDataset[0].size(); // TODO check existence of [0]
//    float mix_factor = time_freq_balance_ * rec_length; // TODO check in time_freq_balance_ is in [0, 1]
//    size_t n = 4; // we skip 2^1 // TODO try 2
//    size_t n_prev = 0;
//    mix_idx = 0;
//    while (true) {
//        if (n > mix_factor) {
//            if (n > rec_length) { // overrun
//                mix_idx = n_prev;
//                break;
//            }
//            if (mix_factor - n_prev > n - mix_factor) // we stick to n_prev or to n, not greater than max index
//                mix_idx = n;
//            else
//                mix_idx = n_prev;
//            break;
//        }
//        n_prev = n;
//        n = n*2; // n is ever degree of 2
//    }

    //std::cout << "\n" << mix_idx << "\n"; // TODO remove

    auto PreEncoded = outer_encode(TrainingDataset);
    for (size_t subband_idx = 0; subband_idx<PreEncoded.size(); ++subband_idx) {
        PCA_models.push_back(metric::PCFA<recType, void>(PreEncoded[subband_idx], n_features_));
        //n_features = n_features_;
    }
}



template <typename recType, typename Metric>
std::vector<std::vector<recType>>
DSPCC<recType, Metric>::outer_encode(
        const std::vector<recType> & Curves
        ) {

    using ElementType = typename recType::value_type;

    std::vector<std::vector<recType>> TimeFreqMixData;
    for (size_t subband_idx = 0; subband_idx<(n_subbands); ++subband_idx) { // TODO update 8
        std::vector<recType> TimeFreqMixSubbandData;
        for (size_t record_idx = 0; record_idx<Curves.size(); ++record_idx) {
            recType rec = {0};
            TimeFreqMixSubbandData.push_back(rec); // TODO optimize
        }
        TimeFreqMixData.push_back(TimeFreqMixSubbandData);
    }

    for (size_t record_idx = 0; record_idx<Curves.size(); ++record_idx) {
        //std::deque<std::vector<ElementType>> current_rec_subbands_timedomain = wavelet::wavedec<ElementType>(Curves[record_idx], 8, 5); // TODO update 8
        //std::deque<std::vector<ElementType>> current_rec_subbands_timedomain = sequential_DWT<ElementType>(Curves[record_idx]); // TODO update 8
        std::stack<size_t> subband_length_local;
        std::deque<std::vector<ElementType>> current_rec_subbands_timedomain = sequential_DWT<ElementType>(Curves[record_idx], subband_length_local, 5, n_subbands);
        if (mix_idx==0) {
            mix_idx = mix_index(current_rec_subbands_timedomain[0].size(), time_freq_balance);
            subband_length = subband_length_local;
        }
        std::deque<std::vector<ElementType>> current_rec_subbands_freqdomain(current_rec_subbands_timedomain);
        metric::apply_DCT_STL(current_rec_subbands_freqdomain, false, mix_idx); // transform and cuts all subbands at once, TODO refactor cutting!!
        for (size_t subband_idx = 0; subband_idx<current_rec_subbands_timedomain.size(); ++subband_idx) {
            recType subband_freqdomain = current_rec_subbands_freqdomain[subband_idx];  // here we possibly drop support of containers other than std::vector
            recType subband_timedomain = current_rec_subbands_timedomain[subband_idx]; // TODO remove intermediate var
            recType subband_mixed;
            subband_mixed.insert(
                        subband_mixed.end(),
                        std::make_move_iterator(subband_freqdomain.begin()),
                        std::make_move_iterator(subband_freqdomain.begin() + mix_idx)
                        );
            subband_mixed.insert(
                        subband_mixed.end(),
                        std::make_move_iterator(subband_timedomain.begin() + mix_idx),
                        std::make_move_iterator(subband_timedomain.end())
                        );
            // TODO mix time and freq properly
            TimeFreqMixData[subband_idx][record_idx] = subband_mixed;
            //TimeFreqMixData[subband_idx].push_back(subband_mixed); // TODO consider

//            std::copy(
//                  subband_mixed.begin(),
//                  subband_mixed.end(),
//                  std::ostream_iterator<int>(std::cout, "\n")
//                );

//            std::cout << record_idx << " " << subband_idx << "\n";
        }
    }
    return TimeFreqMixData;
}


template <typename recType, typename Metric>
std::vector<recType>
DSPCC<recType, Metric>::outer_decode(const std::vector<std::vector<recType>> & TimeFreqMixedData) {

    using ElementType = typename recType::value_type;

    std::vector<recType> Curves;
    for (size_t record_idx = 0; record_idx<TimeFreqMixedData[0].size(); ++record_idx) { // TODO check if [0] element exists
        std::deque<recType> subbands_timedomain;
        std::deque<recType> subbands_freqdomain;
        for (size_t subband_idx = 0; subband_idx<TimeFreqMixedData.size(); ++subband_idx) {
            auto subband_mixed = TimeFreqMixedData[subband_idx][record_idx];
            recType current_subband_freqdomain(subband_mixed.begin(), subband_mixed.begin() + mix_idx);
            recType current_subband_timedomain(subband_mixed.begin() + mix_idx, subband_mixed.end());
            subbands_timedomain.push_back(current_subband_timedomain);
            subbands_freqdomain.push_back(current_subband_freqdomain);
        }
        metric::apply_DCT_STL(subbands_freqdomain, true);
        //std::vector<ElementType> restored_waveform = wavelet::waverec(current_rec_subbands_timedomain, 5);
        //std::vector<ElementType> restored_waveform = wavelet::waverec(current_rec_subbands_freqdomain, 5);
        std::deque<recType> current_rec_subbands_mixed;
        for (size_t subband_idx = 0; subband_idx<TimeFreqMixedData.size(); ++subband_idx) { //TODO optimize!!
            // TODO move to function!!
            recType subband_mixed;
            subband_mixed.insert(
                        subband_mixed.end(),
                        std::make_move_iterator(subbands_freqdomain[subband_idx].begin()),
                        std::make_move_iterator(subbands_freqdomain[subband_idx].begin() + mix_idx) // can be replaced with simply .end()
                        );
            subband_mixed.insert(
                        subband_mixed.end(),
                        std::make_move_iterator(subbands_timedomain[subband_idx].begin()), // concat all vector
                        std::make_move_iterator(subbands_timedomain[subband_idx].end())
                        );
            current_rec_subbands_mixed.push_back(subband_mixed);
        }
        //std::vector<ElementType> restored_waveform = wavelet::waverec(current_rec_subbands_mixed, 5);
        //std::vector<ElementType> restored_waveform = sequential_iDWT(current_rec_subbands_mixed);
        std::stack<size_t> subband_length_copy(subband_length);
        std::vector<ElementType> restored_waveform = sequential_iDWT(current_rec_subbands_mixed, subband_length_copy, 5);
        recType restored_waveform_out;
        for (size_t el_idx = 0; el_idx<restored_waveform.size(); ++el_idx) {
            restored_waveform_out.push_back(restored_waveform[el_idx]);
        }
        Curves.push_back(restored_waveform_out);
    }
    return Curves;
}




template <typename recType, typename Metric>
std::vector<std::vector<recType>>
DSPCC<recType, Metric>::encode(const std::vector<recType> & Data) {
    std::vector<std::vector<recType>> Encoded;
    auto PreEncoded = outer_encode(Data);
    for (size_t subband_idx = 0; subband_idx<PreEncoded.size(); ++subband_idx) {
        auto encoded_subband = PCA_models[subband_idx].encode(PreEncoded[subband_idx]);
        Encoded.push_back(encoded_subband);
    }
    return Encoded; // TODO add rearrangement procedure
}


template <typename recType, typename Metric>
std::vector<recType>
DSPCC<recType, Metric>::decode(const std::vector<std::vector<recType>> & Codes) {
    // TODO add rearrangement procedure
    std::vector<std::vector<recType>> PreDecoded;
    for (size_t subband_idx = 0; subband_idx<Codes.size(); ++subband_idx) {
        auto  decoded_subband = PCA_models[subband_idx].decode(Codes[subband_idx]);
        PreDecoded.push_back(decoded_subband);
    }
    std::vector<recType> Decoded = outer_decode(PreDecoded);
    return Decoded;
}


template <typename recType, typename Metric>
size_t
DSPCC<recType, Metric>::mix_index(size_t length, float time_freq_balance) {
    // computing 2^n value nearest to given time-freq mix factor
    //size_t length = TrainingDataset[0].size(); // TODO check existence of [0]
    float mix_factor = time_freq_balance * length; // TODO check in time_freq_balance_ is in [0, 1]
    size_t n = 4; // we skip 2^1 // TODO try 2
    size_t n_prev = 0;
    size_t mix_index = 0;
    while (true) {
        if (n > mix_factor) {
            if (n > length) { // overrun
                mix_index = n_prev;
                break;
            }
            if (mix_factor - n_prev > n - mix_factor) // we stick to n_prev or to n, not greater than max index
                mix_index = n;
            else
                mix_index = n_prev;
            break;
        }
        n_prev = n;
        n = n*2; // n is ever degree of 2
    }
    return mix_index;
}


}  // namespace metric
