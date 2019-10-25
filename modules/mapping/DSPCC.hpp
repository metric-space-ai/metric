/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_DSPCC_HPP
#define _METRIC_DSPCC_HPP

#include <stack>
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
   * @param time_freq_balance_ - contrinution of time domain data in mixed time-freqency PCFA input, values from 0 to 1
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
class DSPCC {

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
    DSPCC(
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
   * @param time_freq_balance_ - contrinution of time domain data in mixed time-freqency PCFA input, values from 0 to 1
   */
    DSPCC(const std::vector<recType> & TrainingDataset, size_t n_features_ = 1, float time_freq_balance_ = 0.5);


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





}  // namespace metric

#include "DSPCC.cpp"

#endif  // _METRIC_DSPCC_HPP
