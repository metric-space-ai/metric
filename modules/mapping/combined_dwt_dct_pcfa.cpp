
#include "combined_dwt_dct_pcfa.hpp"

#include "../../modules/transform/wavelet.hpp"
#include "../../modules/transform/discrete_cosine.hpp"



namespace metric {


template <typename recType, typename Metric>
PCFA_combined<recType, Metric>::PCFA_combined(
        const std::vector<recType> & TrainingDataset,
        size_t n_features_,
        float time_freq_balance_,
        float DCT_cutoff_
        ) {

    auto PreEncoded = outer_encode(TrainingDataset);
    for (size_t subband_idx = 0; subband_idx<PreEncoded.size(); ++subband_idx) {
        PCA_models.push_back(metric::PCFA<recType, void>(PreEncoded[subband_idx], n_features_));
        //n_features = n_features_;
    }

    // computing 2^n value nearest to given time-freq mix factor
    size_t rec_length = TrainingDataset[0].size(); // TODO check existence of [0]
    float mix_factor = time_freq_balance_ * rec_length; // TODO check in time_freq_balance_ is in [0, 1]
    size_t n = 4; // we skip 2^1 // TODO try 2
    size_t n_prev = 0;
    mix_idx = 0;
    while (true) {
        if (n > mix_factor) {
            if (n > rec_length) { // overrun
                mix_idx = n_prev;
                break;
            }
            if (mix_factor - n_prev > n - mix_factor) // we stick to n_prev or to n, not greater than max index
                mix_idx = n;
            else
                mix_idx = n_prev;
            break;
        }
        n_prev = n;
        n = n*2; // n is ever degree of 2
    }

    std::cout << "\n" << mix_idx << "\n"; // TODO remove
}



template <typename recType, typename Metric>
std::vector<std::vector<recType>>
PCFA_combined<recType, Metric>::outer_encode(
        const std::vector<recType> & Curves
        ) {

    using ElementType = typename recType::value_type;

    std::vector<std::vector<recType>> TimeFreqMixData;
    for (size_t subband_idx = 0; subband_idx<(8+2); ++subband_idx) { // TODO update 8
        std::vector<recType> TimeFreqMixSubbandData;
        for (size_t record_idx = 0; record_idx<Curves.size(); ++record_idx) {
            recType rec = {0};
            TimeFreqMixSubbandData.push_back(rec); // TODO optimize
        }
        TimeFreqMixData.push_back(TimeFreqMixSubbandData);
    }

    for (size_t record_idx = 0; record_idx<Curves.size(); ++record_idx) {
        std::deque<std::vector<ElementType>> current_rec_subbands_timedomain = wavelet::wavedec<ElementType>(Curves[record_idx], 8, 5);
        std::deque<std::vector<ElementType>> current_rec_subbands_freqdomain(current_rec_subbands_timedomain);
        metric::apply_DCT_STL(current_rec_subbands_freqdomain, false); // transform all subbands at once
        for (size_t subband_idx = 0; subband_idx<current_rec_subbands_timedomain.size(); ++subband_idx) {
            recType subband_mixed = current_rec_subbands_timedomain[subband_idx];  // here we possibly drop support of containers oyher than std::vector
            recType subband_freqdomain = current_rec_subbands_freqdomain[subband_idx]; // TODO remove intermediate var
            subband_mixed.insert(
                        subband_mixed.end(),
                        std::make_move_iterator(subband_freqdomain.begin()),
                        std::make_move_iterator(subband_freqdomain.end())
                        );
            // TODO mix time and freq properly
            TimeFreqMixData[subband_idx][record_idx] = subband_mixed;

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
PCFA_combined<recType, Metric>::outer_decode(const std::vector<std::vector<recType>> & TimeFreqMixedData) {

    using ElementType = typename recType::value_type;

    std::vector<recType> Curves;
    for (size_t record_idx = 0; record_idx<TimeFreqMixedData[0].size(); ++record_idx) { // TODO check if [0] element exists
        std::deque<recType> current_rec_subbands_time_domain;
        for (size_t subband_idx = 0; subband_idx<TimeFreqMixedData.size(); ++subband_idx) {
            auto subband_mixed = TimeFreqMixedData[subband_idx][record_idx];
            int time_domain_part_length = subband_mixed.size()/2; // TODO update
            std::vector<ElementType> subband_time_domain(subband_mixed.begin(), subband_mixed.begin() + time_domain_part_length);
            std::vector<ElementType> subband_freq_domain(subband_mixed.begin() + time_domain_part_length, subband_mixed.end());
            // Here we concatenate them instead of taking only time_domain, TODO implement
            current_rec_subbands_time_domain.push_back(subband_time_domain);
        }
        std::vector<ElementType> restored_waveform = wavelet::waverec(current_rec_subbands_time_domain, 5);
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
PCFA_combined<recType, Metric>::encode(const std::vector<recType> & Data) {
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
PCFA_combined<recType, Metric>::decode(const std::vector<std::vector<recType>> & Codes) {
    // TODO add rearrangement procedure
    std::vector<std::vector<recType>> PreDecoded;
    for (size_t subband_idx = 0; subband_idx<Codes.size(); ++subband_idx) {
        auto  decoded_subband = PCA_models[subband_idx].decode(Codes[subband_idx]);
        PreDecoded.push_back(decoded_subband);
    }
    std::vector<recType> Decoded = outer_decode(PreDecoded);
    return Decoded;
}





}  // namespace metric
