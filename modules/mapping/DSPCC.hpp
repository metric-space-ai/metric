/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DSPCC_HPP
#define _METRIC_DSPCC_HPP

#include <stack>
#include <tuple>
#include <cmath> // for log2

#include "../../3rdparty/blaze/Blaze.h"
#include "../../modules/mapping/PCFA.hpp"

namespace metric {



/**
 * @class VibrationFeatureExtractor
 *
 *@brief wrapper for PCFA that combines it with DWT decomposition and DCT pre- and postprocessing
 *
 */
template <typename recType, typename Metric>
class DSPCC {


private:


    template<typename C, int = determine_container_type<C>::code>
    struct determine_recTypeInner // type for internal processing, that can not be Blaze vector
    {
        using type = void;
    };

    template<typename C>
    struct determine_recTypeInner<C, 1> // STL container
    {
        using type = C;
    };


    template<typename C>
    struct determine_recTypeInner<C, 2> // Blaze vector
    {
        using type = std::vector<typename C::ElementType>; // we use STL vector for internal computations
    };



public:

    using value_type = typename determine_element_type<recType>::type;
    using recTypeInner = typename determine_recTypeInner<recType>::type;

    /**
   * @brief
   *
   * @param TrainingDataset - training dataset
   * @param n_features_ - desired number of features of frequency and time PCFAs
   * @param n_subbands_ - (maximum) number of DWT subbands
   * @param time_freq_balance_ - share of frequency domain data in mixed PCFA output codes, values from 0 to 1
   * @param n_top_features_ - number of featores of top PCFA
   */
    DSPCC(
            const std::vector<recType> & TrainingDataset,
            size_t n_features_ = 1,
            size_t n_subbands_ = 4,
            float time_freq_balance_ = 0.5,
            size_t n_top_features_ = 16
            );



    /**
   * @brief computes features for arbitrary waveforms by subbands - first stage of compressing
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    std::vector<std::vector<recTypeInner>> time_freq_PCFA_encode(const std::vector<recTypeInner> & Data);



    /**
   * @brief restores waveforms fromf features computed by time_freq_PCFA_encode
   *
   * @param Codes - compressed codes
   * @return
   */
    std::vector<recTypeInner> time_freq_PCFA_decode(const std::vector<std::vector<recTypeInner>> & Codes);



    /**
   * @brief get features for arbitrary waveforms
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    std::vector<recType> encode(const std::vector<recType> & Data);


    /**
   * @brief restores waveforms
   *
   * @param Codes - compressed codes
   * @return
   */
    std::vector<recType> decode(const std::vector<recType> & Codes);




    std::tuple<std::deque<std::vector<recType>>, std::deque<std::vector<recType>>>
    test_public_wrapper_encode(const std::vector<recType> & Curves) { // TODO remove when tested
        return outer_encode(Curves);
    }

    std::vector<recType>
    test_public_wrapper_decode(const std::tuple<std::deque<std::vector<recType>>, std::deque<std::vector<recType>>> & TimeFreqMixedData) { // TODO remove when tested
      return outer_decode(TimeFreqMixedData);
    }


    /**
   * @brief returns size of subband
   *
   * @return
   */
    size_t get_subband_size() {
        return resulting_subband_length;
    }



private:

    std::vector<metric::PCFA<recTypeInner, Metric>> freq_PCA_models;
    std::vector<metric::PCFA<recTypeInner, Metric>> time_PCA_models;
    std::vector<metric::PCFA<recTypeInner, Metric>> top_PCA_model; // TODO solve initialization issue, remove wrapping vector
    std::stack<size_t> subband_length;
    size_t n_subbands;
    size_t resulting_subband_length;
    float time_freq_balance;
    size_t n_features;
    size_t n_features_freq;
    size_t n_features_time;
    size_t n_top_subbands;
    std::default_random_engine rgen;


    /**
   * @brief conditionally compiled function for selection between outer and inner recType: STL case
   *
   * @param TrainingDataset - training dataset
   * @param n_features_ - desired number of features of frequency and time PCFAs
   * @param n_subbands_ - (maximum) number of DWT subbands
   * @param time_freq_balance_ - share of frequency domain data in mixed PCFA output codes, values from 0 to 1
   * @param n_top_features_ - number of featores of top PCFA
   * @return
   */
    template <typename R>
    typename std::enable_if <
     //DSPCC<recType, Metric>:: template determine_container_type<R>::code == 1,
     determine_container_type<R>::code == 1,
     void
    >::type
    select_train(
            const std::vector<recType> & TrainingDataset,
            size_t n_features_,
            size_t n_subbands_,
            float time_freq_balance_,
            size_t n_top_features_
            );

    /**
   * @brief conditionally compiled function for selection between outer and inner recType: Blaze case
   *
   * @param TrainingDataset - training dataset
   * @param n_features_ - desired number of features of frequency and time PCFAs
   * @param n_subbands_ - (maximum) number of DWT subbands
   * @param time_freq_balance_ - share of frequency domain data in mixed PCFA output codes, values from 0 to 1
   * @param n_top_features_ - number of featores of top PCFA
   * @return
   */
    template <typename R>
    typename std::enable_if <
     determine_container_type<R>::code == 2,
     void
    >::type
    select_train(
            const std::vector<recType> & TrainingDataset,
            size_t n_features_,
            size_t n_subbands_,
            float time_freq_balance_,
            size_t n_top_features_
            );

    /**
   * @brief training procedure called in constructor
   *
   * @param TrainingDataset - training dataset
   * @param n_features_ - desired number of features of frequency and time PCFAs
   * @param n_subbands_ - (maximum) number of DWT subbands
   * @param time_freq_balance_ - share of frequency domain data in mixed PCFA output codes, values from 0 to 1
   * @param n_top_features_ - number of featores of top PCFA
   * @return
   */
    void train(
            const std::vector<DSPCC<recType, Metric>::recTypeInner> & TrainingDataset,
            size_t n_features_,
            size_t n_subbands_,
            float time_freq_balance_,
            size_t n_top_features_
            );

    /**
   * @brief conditionally compiled function for selection betwween outer and inner recType: STL case
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    template <typename R>
    typename std::enable_if <
     determine_container_type<R>::code == 1, // STL case
     std::vector<recType>
    >::type
    select_encode(const std::vector<recType> & Data);

    /**
   * @brief conditionally compiled function for selection betwween outer and inner recType: Blaze case
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    template <typename R>
    typename std::enable_if <
     //DSPCC<recType, Metric>:: template determine_container_type<R>::code == 2, // Blaze case
     determine_container_type<R>::code == 2, // Blaze case
     std::vector<recType>
    >::type
    select_encode(const std::vector<recType> & Data);


    /**
   * @brief conditionally compiled function for selection betwween outer and inner recType: STL case
   *
   * @param Codes - compressed codes
   * @return
   */
    template <typename R>
    typename std::enable_if <
     determine_container_type<R>::code == 1, // STL case
     std::vector<recType>
    >::type
    select_decode(const std::vector<recType> & Codes);

    /**
   * @brief conditionally compiled function for selection betwween outer and inner recType: Blaze case
   *
   * @param Codes - compressed codes
   * @return
   */
    template <typename R>
    typename std::enable_if <
     determine_container_type<R>::code == 2, // Blaze case
     std::vector<recType>
    >::type
    select_decode(const std::vector<recType> & Codes);



    /**
   * @brief get features for arbitrary waveforms by subbands
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    std::vector<std::vector<recTypeInner>> time_freq_PCFA_encode(const std::tuple<std::deque<std::vector<recTypeInner>>, std::deque<std::vector<recTypeInner>>> & PreEncoded);




    /**
   * @brief
   *
   * @param
   * @return
   */
    std::vector<recTypeInner> mixed_code_serialize(const std::vector<std::vector<recTypeInner>> & PCFA_encoded);



    /**
   * @brief
   *
   * @param
   * @return
   */
    std::vector<std::vector<recTypeInner>> mixed_code_deserialize(const std::vector<recTypeInner> & Codes);



    /**
   * @brief apply DWT, DCT to input data, mix wave and spectrum, rearrande by subbands
   *
   * @param Curves - waveforms of same length and format as TrainingDataset
   * @return - recType containers grouped in vectors by subband, which are collected in the single vector
   */
    std::tuple<std::deque<std::vector<recTypeInner>>, std::deque<std::vector<recTypeInner>>> outer_encode(const std::vector<recTypeInner> & Curves);

    /**
   * @brief separates mix wave and spectrum, apply iDCT, reconstructs the subband waveform, applies iDWT, rearrande by records
   *
   * @param TimeFreqMixedData - vector of vectors (per subband) of recType wave-time combined vectors for each compressed record
   * @return - vector of decoded recType curves
   */
    std::vector<recTypeInner> outer_decode(const std::tuple<std::deque<std::vector<recTypeInner>>, std::deque<std::vector<recTypeInner>>> & TimeFreqMixedData);



    /**
   * @brief rearranges data from per-subband to per-record order
   *
   * @param Codes_by_subbands - vector of vectors (per subband) of compressed codes for each record
   * @return - the same compressed codes grouped in vectors by records, all collected in the single vector
   */
    std::vector<std::vector<recType>> rearrange_to_records(const std::vector<recType> & Codes_by_subbands);

    /**
   * @brief rearranges data from record-major to per-subband order
   *
   * @param Codes_by_records - compressed codes grouped in vectors by records, all collected in the single vector
   * @return - vector of vectors (per subband) of compressed codes for each record
   */
    std::vector<std::vector<recType>> rearrange_to_subbands(const std::vector<recType> & Codes_by_records);


    /**
   * @brief determines break index using given length of the subband waveform
   *
   * @param length - length of the filtered suband waveform
   * @return - index to be used to break the subband waveform apart for separate time domain and frequency domain compression
   */
    size_t mix_index(size_t length, float time_freq_balance);


    /**
   * @brief
   *
   * @param
   * @return
   */
    size_t subband_size(size_t original_size, size_t depth, size_t wavelet_length=10);  // hardcoded DB5 wavelet!! TODO update

    /**
   * @brief
   *
   * @param
   * @return
   */
    size_t original_size(size_t subband_size, size_t depth, size_t wavelet_length=10);



};






}  // namespace metric

#include "DSPCC.cpp"

#endif  // _METRIC_DSPCC_HPP
