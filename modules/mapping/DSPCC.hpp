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




// ------------------------------------------
// old version of DSPCC with cutting and concatenation



/**
 * @class VibrationFeatureExtractor
 * 
 *@brief wrapper for PCFA that combines it with DWT decomposition and optional DCT pre- and postprocessing
 * 
 */
template <typename recType, typename Metric>
class DSPCC1 {

public:
    /**
   * @brief Construct a new VibrationFeatureExtractor object with the DCT step
   * 
   * @param TrainingDataset - Blaze matrix with timeseries in columns
   * @param n_features_ - desired number of features to extract
   * @param n_subbands_ - maximum number of DWT subbands
   * @param time_freq_balance_ - contrinution of time domain data in mixed time-freqency PCFA input, values from 0 to 1
   * @param DCT_cutoff  - the share of waveform allowed to be cut off when making the length of waveform equal to the degree of 2
   */
    DSPCC1(
            const std::vector<recType> & TrainingDataset,
            size_t n_features_ = 1,
            size_t n_subbands_ = 4,
            float time_freq_balance_ = 0.5,
            float DCT_cutoff_ = 0
            );

    /**
   * @brief Construct a new VibrationFeatureExtractor object without DCT step
   *
   * @param TrainingDataset - Blaze matrix with timeseries in columns
   * @param n_features_ - desired number of features to extract
   * @param time_freq_balance_ - contribution of time domain data in mixed time-freqency PCFA input, values from 0 to 1
   */
    DSPCC1(const std::vector<recType> & TrainingDataset, size_t n_features_ = 1, float time_freq_balance_ = 0.5);


    /**
   * @brief get features for arbitrary waveforms
   * 
   * @param Data - waveforms of same length and format as TrainingDataset 
   * @return
   */
    std::vector<std::vector<recType>> encode(const std::vector<recType> & Data);


    /**
   * @brief restores waveforms
   *
   * @param Codes - compressed codes
   * @return
   */
    std::vector<recType> decode(const std::vector<std::vector<recType>> & Codes);



    std::vector<std::vector<recType>> test_public_wrapper_encode(const std::vector<recType> & Curves) { // TODO remove when tested
        return outer_encode(Curves);
    }

    std::vector<recType> test_public_wrapper_decode(const std::vector<std::vector<recType>> & TimeFreqMixedData) { // TODO remove when tested
      return outer_decode(TimeFreqMixedData);
    }

    size_t get_mix_idx() {
        return mix_idx;
    }



private:
    std::vector<metric::PCFA<recType, Metric>> PCA_models;
    std::stack<size_t> subband_length;
    size_t n_subbands;
    size_t mix_idx;
    float time_freq_balance;
    //size_t n_features;
    std::default_random_engine rgen;


    /**
   * @brief apply DWT, DCT to input data, mix wave and spectrum, rearrande by subbands
   *
   * @param Curves - waveforms of same length and format as TrainingDataset
   * @return - recType containers grouped in vectors by subband, which are collected in the single vector
   */
    std::vector<std::vector<recType>> outer_encode(const std::vector<recType> & Curves);

    /**
   * @brief separates mix wave and spectrum, apply iDCT, reconstructs the subband waveform, applies iDWT, rearrande by records
   *
   * @param TimeFreqMixedData - vector of vectors (per subband) of recType wave-time combined vectors for each compressed record
   * @return - vector of decoded recType curves
   */
    std::vector<recType> outer_decode(const std::vector<std::vector<recType>> & TimeFreqMixedData);



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

};


/**
* @brief returns a new VibrationFeatureExtractor object with auto type deduction
*
* @param TrainingDataset - Blaze matrix with timeseries in columns
* @param n_features - desired number of features to extract
* @param DCT_cutoff  - float value that controls the DCT step: if negative, skip this step, if in [0..1), the share of waveform allowed to be cut off when making the length of waveform equal to the degree of 2
*/
template <template <typename, typename> class Container, typename ValueType, typename Allocator>
DSPCC1<Container<ValueType, Allocator>, void> PCFA_combined_factory(const Container<ValueType, Allocator> & TrainingDataset, size_t n_features = 1, float time_freq_balance = 0.5, float DCT_cutoff = -1) {
    return DSPCC1<Container<ValueType, Allocator>, void>(TrainingDataset, n_features, time_freq_balance, DCT_cutoff);
}




// ------------------------------------------
// version of DSPCC with no cutting




/**
 * @class VibrationFeatureExtractor
 *
 *@brief wrapper for PCFA that combines it with DWT decomposition and optional DCT pre- and postprocessing
 *
 */
template <typename recType, typename Metric>
class DSPCC_single_PCFA {

public:
    /**
   * @brief Construct a new VibrationFeatureExtractor object with the DCT step
   *
   * @param TrainingDataset - Blaze matrix with timeseries in columns
   * @param n_features_ - desired number of features to extract
   * @param n_subbands_ - maximum number of DWT subbands
   * @param time_freq_balance_ - contribution of time domain data in mixed time-freqency PCFA input, values from 0 to 1
   * @param DCT_cutoff  - the share of waveform allowed to be cut off when making the length of waveform equal to the degree of 2
   */
    DSPCC_single_PCFA(
            const std::vector<recType> & TrainingDataset,
            size_t n_features_ = 1,
            size_t n_subbands_ = 4,
            float time_freq_balance_ = 0.5,
            float DCT_cutoff_ = 0
            );

    /**
   * @brief Construct a new VibrationFeatureExtractor object without DCT step
   *
   * @param TrainingDataset - Blaze matrix with timeseries in columns
   * @param n_features_ - desired number of features to extract
   * @param time_freq_balance_ - contribution of time domain data in mixed time-freqency PCFA input, values from 0 to 1
   */
    DSPCC_single_PCFA(const std::vector<recType> & TrainingDataset, size_t n_features_ = 1, float time_freq_balance_ = 0.5);


    /**
   * @brief get features for arbitrary waveforms
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    std::vector<std::vector<recType>> encode(const std::vector<recType> & Data);


    /**
   * @brief restores waveforms
   *
   * @param Codes - compressed codes
   * @return
   */
    std::vector<recType> decode(const std::vector<std::vector<recType>> & Codes);



    std::vector<std::vector<recType>> test_public_wrapper_encode(const std::vector<recType> & Curves) { // TODO remove when tested
        return outer_encode(Curves);
    }

    std::vector<recType> test_public_wrapper_decode(const std::vector<std::vector<recType>> & TimeFreqMixedData) { // TODO remove when tested
      return outer_decode(TimeFreqMixedData);
    }

    size_t get_crop_idx() {
        return crop_idx;
    }



private:
    std::vector<metric::PCFA<recType, Metric>> PCA_models;
    std::stack<size_t> subband_length;
    size_t n_subbands;
    size_t crop_idx;
    float time_freq_balance;
    //size_t n_features;
    std::default_random_engine rgen;


    /**
   * @brief apply DWT, DCT to input data, mix wave and spectrum, rearrande by subbands
   *
   * @param Curves - waveforms of same length and format as TrainingDataset
   * @return - recType containers grouped in vectors by subband, which are collected in the single vector
   */
    std::vector<std::vector<recType>> outer_encode(const std::vector<recType> & Curves);

    /**
   * @brief separates mix wave and spectrum, apply iDCT, reconstructs the subband waveform, applies iDWT, rearrande by records
   *
   * @param TimeFreqMixedData - vector of vectors (per subband) of recType wave-time combined vectors for each compressed record
   * @return - vector of decoded recType curves
   */
    std::vector<recType> outer_decode(const std::vector<std::vector<recType>> & TimeFreqMixedData);



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



    size_t subband_size(size_t original_size, size_t depth, size_t wavelet_length=10);  // hardcoded DB5 wavelet!! TODO update

    size_t original_size(size_t subband_size, size_t depth, size_t wavelet_length=10);



};







// ------------------------------------------
// version of DSPCC with post-PCFA mixing





/**
 * @class VibrationFeatureExtractor
 *
 *@brief wrapper for PCFA that combines it with DWT decomposition and optional DCT pre- and postprocessing
 *
 */
template <typename recType, typename Metric>
class DSPCC {


private:


    template <typename>
    struct determine_container_type  // checks whether container is STL container (1) or Blaze vector (2)
    {
        constexpr static int code = 0;
    };

    template <template <typename, typename> class Container, typename ValueType, typename Allocator>
    struct determine_container_type<Container<ValueType, Allocator>>
    {
        constexpr static int code = 1;
    };

    template <template <typename, bool> class Container, typename ValueType, bool F>
    struct determine_container_type<Container<ValueType, F>>
    {
        constexpr static int code = 2;
    };



    template<typename C, int = determine_container_type<C>::code>
    struct determine_element_type  // determines type of element both for STL containers and Blaze vectors
    {
        using type = void;
    };

    template<typename C>
    struct determine_element_type<C, 1>
    {
        using type = typename C::value_type;
    };

    template<typename C>
    struct determine_element_type<C, 2>
    {
        using type = typename C::ElementType;
    };



    template<typename C, int = determine_container_type<C>::code>
    struct determine_recTypeInner
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




//    /**
//   * @brief Construct a new VibrationFeatureExtractor object without DCT step
//   *
//   * @param TrainingDataset - Blaze matrix with timeseries in columns
//   * @param n_features_ - desired number of features of frequency and time PCFAs
//   * @param time_freq_balance_ - contrinution of time domain data in mixed PCFA output codes, values from 0 to 1
//   * @param n_top_features_ - number of featores of top PCFA
//   */
//    DSPCC(const std::vector<recType> & TrainingDataset, size_t n_features_ = 1, float time_freq_balance_ = 0.5);


    /**
   * @brief get features for arbitrary waveforms by subbands
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    std::vector<std::vector<recType>> time_freq_PCFA_encode(const std::vector<recType> & Data);


    /**
   * @brief get features for arbitrary waveforms by subbands
   *
   * @param Data - waveforms of same length and format as TrainingDataset
   * @return
   */
    std::vector<std::vector<recType>> time_freq_PCFA_encode(const std::tuple<std::deque<std::vector<recType>>, std::deque<std::vector<recType>>> & PreEncoded);



    /**
   * @brief restores waveforms
   *
   * @param Codes - compressed codes
   * @return
   */
    std::vector<recType> time_freq_PCFA_decode(const std::vector<std::vector<recType>> & Codes);




    /**
   * @brief
   *
   * @param
   * @return
   */
    std::vector<recType> mixed_code_serialize(const std::vector<std::vector<recType>> & PCFA_encoded);



    /**
   * @brief
   *
   * @param
   * @return
   */
    std::vector<std::vector<recType>> mixed_code_deserialize(const std::vector<recType> & Codes);




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
    float time_freq_balance; // TODO remove
    size_t n_features;
    size_t n_features_freq;
    size_t n_features_time;
    size_t n_top_subbands;
    std::default_random_engine rgen;



    template <typename R>
    typename std::enable_if <
     DSPCC<recType, Metric>:: template determine_container_type<R>::code == 1,
     void
    >::type
    select_train(
            const std::vector<recType> & TrainingDataset,
            size_t n_features_ = 1, // TODO remove defaults everywhere except ctor
            size_t n_subbands_ = 4,
            float time_freq_balance_ = 0.5,
            size_t n_top_features_ = 16
            );


    template <typename R>
    typename std::enable_if <
     DSPCC<recType, Metric>:: template determine_container_type<R>::code == 2,
     void
    >::type
    select_train(
            const std::vector<recType> & TrainingDataset,
            size_t n_features_ = 1,
            size_t n_subbands_ = 4,
            float time_freq_balance_ = 0.5,
            size_t n_top_features_ = 16
            );



    void train(
            const std::vector<DSPCC<recType, Metric>::recTypeInner> & TrainingDataset,
            size_t n_features_ = 1,
            size_t n_subbands_ = 4,
            float time_freq_balance_ = 0.5,
            size_t n_top_features_ = 16
            );


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



    size_t subband_size(size_t original_size, size_t depth, size_t wavelet_length=10);  // hardcoded DB5 wavelet!! TODO update

    size_t original_size(size_t subband_size, size_t depth, size_t wavelet_length=10);



};






}  // namespace metric

#include "DSPCC.cpp"

#endif  // _METRIC_DSPCC_HPP
