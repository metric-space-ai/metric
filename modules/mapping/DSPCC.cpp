
#include "DSPCC.hpp"

#include "modules/transform/wavelet.hpp"
#include "modules/transform/discrete_cosine.hpp"



namespace metric {





// ------------------------------------------
// common functions


// recursive split for arbitrary depth
// TODO consider creating special class for DWT split in tree order (with stack encaplulated)


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


//template <typename T>
//std::deque<std::vector<T>>
//sequential_DWT( // old overload
//        std::vector<T> x,
//        std::stack<size_t> & subband_length,
//        int wavelet_type,
//        size_t subbands_num
//        ) {
//    std::deque<std::vector<T>> deque_x = {x};
//    return DWT_split(deque_x, subband_length, wavelet_type, subbands_num);
//}



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
    OuterContainer<InnerContainer, OuterAllocator> deque_x = {x};
    return DWT_split(deque_x, subband_length, wavelet_type, subbands_num);
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
    OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> deque_out = DWT_unsplit(in, subband_length, wavelet_type);
    return deque_out[0];
}








// ------------------------------------------
// class DSPCC




template <typename RecType, typename Metric>
DSPCC<RecType, Metric>::DSPCC(
        const std::vector<RecType> & TrainingDataset,
        size_t n_features_,
        size_t n_subbands_,
        float time_freq_balance_,
        size_t n_top_features_
        ) {
    select_train<RecType>(TrainingDataset, n_features_, n_subbands_, time_freq_balance_, n_top_features_);
}




template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if <
 determine_container_type<R>::code == 1,
 void
>::type
DSPCC<RecType, Metric>::select_train(
        const std::vector<RecType> & TrainingDataset,
        size_t n_features_,
        size_t n_subbands_,
        float time_freq_balance_,
        size_t n_top_features_
        ) {
    train(TrainingDataset, n_features_, n_subbands_, time_freq_balance_, n_top_features_);
}




template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if <
 determine_container_type<R>::code == 2,
 void
>::type
DSPCC<RecType, Metric>::select_train(
        const std::vector<RecType> & TrainingDataset,
        size_t n_features_,
        size_t n_subbands_,
        float time_freq_balance_,
        size_t n_top_features_
        ) {

    using ValueType = DSPCC<RecType, Metric>::value_type;

    // convert from Blaze to STL  // TODO move to separate private method
    std::vector<RecTypeInner> ConvertedDataset;
    for (size_t i=0; i<TrainingDataset.size(); ++i) {
        RecTypeInner line;
        for (size_t j=0; j<TrainingDataset[i].size(); ++j) {
            line.push_back(TrainingDataset[i][j]);
        }
        ConvertedDataset.push_back(line);
    }

    train(ConvertedDataset, n_features_, n_subbands_, time_freq_balance_, n_top_features_);
}




template <typename RecType, typename Metric>
void DSPCC<RecType, Metric>::train(
        const std::vector<DSPCC<RecType, Metric>::RecTypeInner> & TrainingDataset,
        size_t n_features_,
        size_t n_subbands_,
        float time_freq_balance_,
        size_t n_top_features_
        ) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    time_freq_balance = time_freq_balance_;
    resulting_subband_length = 0;
    n_features = n_features_; // number of features selected from both PCFAs
    n_features_freq = std::round(n_features_*time_freq_balance_);
    n_features_time = n_features_ - n_features_freq;
    for (size_t n = 2; n<=n_subbands_; n = n*2)
        n_subbands = n;
    n_top_subbands = n_top_features_;

    auto PreEncoded = outer_encode(TrainingDataset);
    for (size_t subband_idx = 0; subband_idx<std::get<0>(PreEncoded).size(); ++subband_idx) {
        freq_PCA_models.push_back(metric::PCFA<RecTypeInner, void>(std::get<0>(PreEncoded)[subband_idx], n_features_freq));
        time_PCA_models.push_back(metric::PCFA<RecTypeInner, void>(std::get<1>(PreEncoded)[subband_idx], n_features_time));
    }
    std::vector<std::vector<RecTypeInner>> time_freq_PCFA_encoded = time_freq_PCFA_encode(PreEncoded);
    std::vector<RecTypeInner> series = mixed_code_serialize(time_freq_PCFA_encoded);
    top_PCA_model.push_back(metric::PCFA<RecTypeInner, void>(series, n_top_subbands));
}


template <typename RecType, typename Metric>
std::tuple<std::deque<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>, std::deque<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>>
DSPCC<RecType, Metric>::outer_encode(
        const std::vector<DSPCC<RecType, Metric>::RecTypeInner> & Curves
        ) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;
    using ElementType = typename RecTypeInner::value_type;

    std::deque<std::vector<RecTypeInner>> FreqData;
    std::deque<std::vector<RecTypeInner>> TimeData;
    for (size_t subband_idx = 0; subband_idx<(n_subbands); ++subband_idx) {
        std::vector<RecTypeInner> SubbandData;
        for (size_t record_idx = 0; record_idx<Curves.size(); ++record_idx) {
            RecTypeInner rec = {0};
            SubbandData.push_back(rec); // TODO optimize
        }
        TimeData.push_back(SubbandData);
        FreqData.push_back(SubbandData);
    }


    // compute size and crop input
    size_t depth = (size_t)std::floor(std::log2(n_subbands));
    size_t max_subband_size = subband_size(Curves[0].size(), depth); // TODO check if not empty
    size_t appropriate_subband_size = crop_index(max_subband_size, 1);
    size_t crop_size = original_size(appropriate_subband_size, depth);

    for (size_t record_idx = 0; record_idx<Curves.size(); ++record_idx) {
        std::stack<size_t> subband_length_local;
        RecTypeInner cropped_record(Curves[record_idx].begin(), Curves[record_idx].begin() + crop_size);

        std::deque<RecTypeInner> current_rec_subbands_timedomain = sequential_DWT<std::deque, RecTypeInner, std::allocator<RecTypeInner>>(cropped_record, subband_length_local, 5, n_subbands); // TODO replace 5!! // TODO support different RecType types
        if (resulting_subband_length==0) { // only during the first run
            resulting_subband_length = current_rec_subbands_timedomain[0].size();
            subband_length = subband_length_local;
        }
        std::deque<RecTypeInner> current_rec_subbands_freqdomain(current_rec_subbands_timedomain);
        metric::apply_DCT_STL(current_rec_subbands_freqdomain, false, resulting_subband_length); // transform all subbands at once (only first mix_idx values are replaced, the rest is left unchanged!), TODO refactor cutting!!
        for (size_t subband_idx = 0; subband_idx<current_rec_subbands_timedomain.size(); ++subband_idx) {
            TimeData[subband_idx][record_idx] = current_rec_subbands_timedomain[subband_idx];
            FreqData[subband_idx][record_idx] = current_rec_subbands_freqdomain[subband_idx];
        }
    }
    return std::make_tuple(FreqData, TimeData);
}


template <typename RecType, typename Metric>
std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>
DSPCC<RecType, Metric>::outer_decode(const std::tuple<std::deque<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>, std::deque<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>> & TimeFreqData) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::deque<std::vector<RecTypeInner>> FreqData = std::get<0>(TimeFreqData);
    std::deque<std::vector<RecTypeInner>> TimeData = std::get<1>(TimeFreqData);

    std::vector<RecTypeInner> Curves;
    for (size_t record_idx = 0; record_idx<TimeData[0].size(); ++record_idx) { // TODO check if [0] element exists
        std::vector<RecTypeInner> subbands_freqdomain;
        std::vector<RecTypeInner> subbands_timedomain;
        for (size_t subband_idx = 0; subband_idx<TimeData.size(); ++subband_idx) {
            subbands_timedomain.push_back(TimeData[subband_idx][record_idx]);
            subbands_freqdomain.push_back(FreqData[subband_idx][record_idx]);
        }
        metric::apply_DCT_STL(subbands_freqdomain, true);

        std::stack<size_t> subband_length_copy(subband_length);
        RecTypeInner restored_waveform_freq = sequential_iDWT(subbands_freqdomain, subband_length_copy, 5);
        subband_length_copy = subband_length;
        RecTypeInner restored_waveform_time = sequential_iDWT(subbands_timedomain, subband_length_copy, 5);
        RecTypeInner restored_waveform_out;
        for (size_t el_idx = 0; el_idx<restored_waveform_freq.size(); ++el_idx) {
            restored_waveform_out.push_back( (restored_waveform_freq[el_idx]*time_freq_balance + restored_waveform_time[el_idx]*(1 - time_freq_balance)) );
        }
        Curves.push_back(restored_waveform_out);
    }
    return Curves;
}





template <typename RecType, typename Metric>
std::vector<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>
DSPCC<RecType, Metric>::time_freq_PCFA_encode(const std::vector<typename DSPCC<RecType, Metric>::RecTypeInner> & Data) {

    return time_freq_PCFA_encode(outer_encode(Data));
}


template <typename RecType, typename Metric>
std::vector<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>
DSPCC<RecType, Metric>::time_freq_PCFA_encode(const std::tuple<std::deque<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>, std::deque<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>> & PreEncoded) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::vector<std::vector<RecTypeInner>> Encoded;
    for (size_t subband_idx = 0; subband_idx<std::get<0>(PreEncoded).size(); ++subband_idx) {
        auto freq_encoded_subband = freq_PCA_models[subband_idx].encode(std::get<0>(PreEncoded)[subband_idx]);
        auto time_encoded_subband = time_PCA_models[subband_idx].encode(std::get<1>(PreEncoded)[subband_idx]);
        // here we crop and concatenate codes
        std::vector<RecTypeInner> encoded_subband;
        for (size_t record_idx = 0; record_idx<freq_encoded_subband.size(); ++record_idx)  {
            RecTypeInner mixed_codes;
            for (size_t el_idx = 0; el_idx<n_features_freq; ++el_idx) {
                mixed_codes.push_back(freq_encoded_subband[record_idx][el_idx]);
            }
            for (size_t el_idx = 0; el_idx<n_features_time; ++el_idx) {
                mixed_codes.push_back(time_encoded_subband[record_idx][el_idx]); // we concatenate all features to single vector
            }
            encoded_subband.push_back(mixed_codes);
        }
        Encoded.push_back(encoded_subband);
    }
    return Encoded;
}


template <typename RecType, typename Metric>
std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>
DSPCC<RecType, Metric>::time_freq_PCFA_decode(const std::vector<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>> & Codes) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::deque<std::vector<RecTypeInner>> FreqData;
    std::deque<std::vector<RecTypeInner>> TimeData;
    for (size_t subband_idx = 0; subband_idx<Codes.size(); ++subband_idx) { // divide each vector of codes into freq and time parts and rearrange data by subbands
        std::vector<RecTypeInner> freq_codes;
        std::vector<RecTypeInner> time_codes;
        for (size_t record_idx = 0; record_idx<Codes[subband_idx].size(); ++record_idx) {
            RecTypeInner freq_code_part(Codes[subband_idx][record_idx].begin(), Codes[subband_idx][record_idx].begin() + n_features_freq);
            RecTypeInner time_code_part(Codes[subband_idx][record_idx].begin() + n_features_freq, Codes[subband_idx][record_idx].end());
            freq_codes.push_back(freq_code_part);
            time_codes.push_back(time_code_part);
        }
        auto decoded_subband_freq = freq_PCA_models[subband_idx].decode(freq_codes);
        auto decoded_subband_time = time_PCA_models[subband_idx].decode(time_codes);
        FreqData.push_back(decoded_subband_freq);
        TimeData.push_back(decoded_subband_time);
    }
    return outer_decode(std::make_tuple(FreqData, TimeData));
}


template <typename RecType, typename Metric>
std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>
DSPCC<RecType, Metric>::mixed_code_serialize(const std::vector<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>> & PCFA_encoded) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;
    using ElementType = typename RecTypeInner::value_type;

    std::vector<RecTypeInner> serialized_dataset;
    for (size_t record_idx = 0; record_idx<PCFA_encoded[0].size(); ++record_idx) {
        RecTypeInner serialized_record;
        for (size_t subband_idx = 0; subband_idx<PCFA_encoded.size(); ++subband_idx) {
            serialized_record.insert(
                        serialized_record.end(),
                        std::make_move_iterator(PCFA_encoded[subband_idx][record_idx].begin()),
                        std::make_move_iterator(PCFA_encoded[subband_idx][record_idx].end())
                        );
        }
        serialized_dataset.push_back(serialized_record);
    }
    return serialized_dataset;
}




template <typename RecType, typename Metric>
std::vector<std::vector<typename DSPCC<RecType, Metric>::RecTypeInner>>
DSPCC<RecType, Metric>::mixed_code_deserialize(const std::vector<typename DSPCC<RecType, Metric>::RecTypeInner> & Codes) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::vector<std::vector<RecTypeInner>> deserialized;
    for (size_t subband_idx = 0; subband_idx<(n_subbands); ++subband_idx) {
        std::vector<RecTypeInner> SubbandData;
        for (size_t record_idx = 0; record_idx<Codes.size(); ++record_idx) {
            RecTypeInner rec = {0};
            SubbandData.push_back(rec); // TODO optimize
        }
        deserialized.push_back(SubbandData);
    }

    for (size_t record_idx = 0; record_idx<Codes.size(); ++record_idx) {
        size_t current_idx = 0;
        for (size_t subband_idx = 0; subband_idx<freq_PCA_models.size(); ++subband_idx) {
            RecTypeInner mixed_code(Codes[record_idx].begin() + current_idx, Codes[record_idx].begin() + current_idx + n_features_freq);
            current_idx += n_features_freq;
            mixed_code.insert(mixed_code.end(), Codes[record_idx].begin() + current_idx, Codes[record_idx].begin() + current_idx + n_features_time);
            current_idx += n_features_time;
            deserialized[subband_idx][record_idx] = mixed_code;
        }
    }
    return deserialized;
}



template <typename RecType, typename Metric>
std::vector<RecType>
DSPCC<RecType, Metric>::encode(const std::vector<RecType> & Data) {

    return select_encode<RecType>(Data);
}



template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if <
 determine_container_type<R>::code == 1, // STL case
 std::vector<RecType>
>::type
DSPCC<RecType, Metric>::select_encode(const std::vector<RecType> & Data) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::vector<RecTypeInner> Codes;
    std::vector<std::vector<RecTypeInner>> time_freq_PCFA_encoded = time_freq_PCFA_encode(Data);
    std::vector<RecTypeInner> series = mixed_code_serialize(time_freq_PCFA_encoded);
    return top_PCA_model[0].encode(series);
}




template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if <
 determine_container_type<R>::code == 2, // Blaze vector case
 std::vector<RecType>
>::type
DSPCC<RecType, Metric>::select_encode(const std::vector<RecType> & Data) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::vector<RecTypeInner> ConvertedData;
    for (size_t i=0; i<Data.size(); ++i) {
        RecTypeInner line;
        for (size_t j=0; j<Data[i].size(); ++j) {
            line.push_back(Data[i][j]);
        }
        ConvertedData.push_back(line);
    }

    std::vector<RecTypeInner> Codes;
    std::vector<std::vector<RecTypeInner>> time_freq_PCFA_encoded = time_freq_PCFA_encode(ConvertedData);
    std::vector<RecTypeInner> series = mixed_code_serialize(time_freq_PCFA_encoded);
    auto pre_output =  top_PCA_model[0].encode(series);

    std::vector<RecType> output;

    // convert back
    for (size_t i=0; i<pre_output.size(); ++i) {
        RecType line(pre_output[i].size());
        for (size_t j=0; j<pre_output[i].size(); ++j) {
            line[j] = pre_output[i][j];
        }
        output.push_back(line);
    }

    return output;
}






template <typename RecType, typename Metric>
std::vector<RecType>
DSPCC<RecType, Metric>::decode(const std::vector<RecType> & Codes) {

    return select_decode<RecType>(Codes);
}



template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if <
 determine_container_type<R>::code == 1, // STL case
 std::vector<RecType>
>::type
DSPCC<RecType, Metric>::select_decode(const std::vector<RecType> & Codes) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::vector<std::vector<RecTypeInner>> deserialized = mixed_code_deserialize(top_PCA_model[0].decode(Codes));
    return time_freq_PCFA_decode(deserialized);
}


template <typename RecType, typename Metric>
template <typename R>
typename std::enable_if <
 determine_container_type<R>::code == 2, // Blaze case
 std::vector<RecType>
>::type
DSPCC<RecType, Metric>::select_decode(const std::vector<RecType> & Codes) {

    using RecTypeInner = DSPCC<RecType, Metric>::RecTypeInner;

    std::vector<RecTypeInner> ConvertedCodes;
    for (size_t i=0; i<Codes.size(); ++i) {
        RecTypeInner line;
        for (size_t j=0; j<Codes[i].size(); ++j) {
            line.push_back(Codes[i][j]);
        }
        ConvertedCodes.push_back(line);
    }

    std::vector<std::vector<RecTypeInner>> deserialized = mixed_code_deserialize(top_PCA_model[0].decode(ConvertedCodes));
    auto pre_output = time_freq_PCFA_decode(deserialized);

    std::vector<RecType> output;

    // convert back
    for (size_t i=0; i<pre_output.size(); ++i) {
        RecType line(pre_output[i].size());
        for (size_t j=0; j<pre_output[i].size(); ++j) {
            line[j] = pre_output[i][j];
        }
        output.push_back(line);
    }

    return output;

}





template <typename RecType, typename Metric>
size_t
DSPCC<RecType, Metric>::crop_index(size_t length, float crop_share) {
    // computing 2^n value nearest to given share value
    float crop_factor = crop_share * length; // TODO check in time_freq_balance_ is in [0, 1]
    size_t n = 4; // we skip 2^1
    size_t n_prev = 0;
    size_t mix_index = 0;
    while (true) {
        if (n > crop_factor) {
            if (n > length) { // overrun
                mix_index = n_prev;
                break;
            }
            if (crop_factor - n_prev > n - crop_factor) // we stick to n_prev or to n, not greater than max index
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



//template <typename RecType, typename Metric>
//size_t
//DSPCC<RecType, Metric>::subband_size(size_t original_size, size_t depth, size_t wavelet_length) { // rounding issue
//    size_t n = 1;
//    float sum = 0;
//    for (size_t i=1; i<=depth; ++i){
//        n = n*2;
//        sum += (wavelet_length - 2)/(float)n; // -2 instead of -1 because of floor rounding within cast
//    }
//    return original_size/(float)n + sum;
//}

template <typename RecType, typename Metric>
size_t
DSPCC<RecType, Metric>::subband_size(size_t original_size, size_t depth, size_t wavelet_length) {
    size_t sz = original_size;
    for (size_t i=1; i<=depth; ++i){
        sz = (sz + wavelet_length - 1)/2.0;
    }
    return sz;
}

template <typename RecType, typename Metric>
size_t
DSPCC<RecType, Metric>::original_size(size_t subband_size, size_t depth, size_t wavelet_length) {
    size_t n = 1;
    float sum = 0;
    for (size_t i=1; i<=depth; ++i){
        n = n*2;
        sum += (wavelet_length - 2)/(float)n;
    }
    return n*(subband_size - sum);
}







}  // namespace metric
