/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2021 Panda Team
*/

#ifndef _METRIC_ESN_SWITCH_DETECTOR_HPP
#define _METRIC_ESN_SWITCH_DETECTOR_HPP


#include "ESN.hpp"


/**
 * @class SwitchPredictor - trainable estimator of On-Off and Off-On switches in 3-dimensional timeseries.
 */
template <typename value_type>
class SwitchPredictor {

public:

    /**
     * @brief create & train model using Blaze dynamic matrix
     *
     * @param training_data - 3-dimensional timeseries, samples in rows
     * @param labels - matrix with single column of label values: -1, 0 or 1.
     * Number of rows should be the same as in training_dataset
     */
    SwitchPredictor(
            const blaze::DynamicMatrix<value_type> & training_data,
            const blaze::DynamicMatrix<value_type> & labels,
            const size_t wnd_size_ = 15,
            const size_t cmp_wnd_sz_ = 80, //150,
            const size_t washout_ = 500, //2500,
            const value_type contrast_threshold_ = 0.2, //0.3,
            const value_type alpha_ = 0.4, //0.1,
            const value_type beta_ = 0.5
            );

    /**
     * @brief create & train model using vectors of STL containers
     *
     * @param training_data - timeseries, each RecType contains 3 values and represents a sample
     * @param labels - vector of containers with single label value in each: -1, 0 or 1.
     * Length should be equal to the length of training_dataset
     */
    template <typename RecType>  // to be deduced
    SwitchPredictor(
            const std::vector<RecType> & training_data,
            const std::vector<RecType> & labels,
            const size_t wnd_size_ = 15,
            const size_t cmp_wnd_sz_ = 80, //150,
            const size_t washout_ = 500, //2500,
            const value_type contrast_threshold_ = 0.2, //0.3,
            const value_type alpha_ = 0.4, //0.1,
            const value_type beta_ = 0.5
            );

    /**
     * @brief load trained model from file
     *
     * @param filename - file with Blaze image of the NN, saved by ::save(...) method.
     */
    SwitchPredictor(const std::string & filename);

    /**
     * @brief estimates switches in offline timeseries
     *
     * @param dataset - 3-dimensional timeseries with sample in rows
     * @return column of -1, 0 or 1 values, where -1 and 1 represent On-Off and Off-On switches respectively
     */
    blaze::DynamicMatrix<value_type> encode(const blaze::DynamicMatrix<value_type> & dataset);

    /**
     * @brief estimates switches in offline timeseries
     *
     * @param dataset - vector of 3-dimensional samples
     * @return column of -1, 0 or 1 values, where -1 and 1 represent On-Off and Off-On switches respectively
     */
    template <typename RecType>
    std::vector<value_type> encode(const std::vector<RecType> & dataset);

    /**
     * @brief encode_raw - adds the passed slice to buffer and encodes the other slice of the same size taken from buffer
     * at 150 samples in the past (before last 150 samples)
     * @param indices - vector of int timestamps of the same size as the input slice
     * @param dataset - the input timeseries, vector of samples (triplets)
     * @return - estimation for samples before 150 last ones,
     * tuple: vector of indices related to the output extimations, and vector of estimated switches,
     * encoded as: 0 - no switch. -1 - On-Off switch, 1 - Off-On switch
     * If buffer size is not enough for estimation, the output is empty of of less size than input
     */
    std::tuple<std::vector<unsigned long long int>, std::vector<value_type>> encode_raw(
            const std::vector<unsigned long long int> & indices,
            const std::vector<std::vector<value_type>> & dataset
            );

    /**
     * @brief encode - adds the passed slice to buffer and encodes the other slice of the same size taken from buffer
     * at 150 samples in the past (before last 150 samples)
     * @param indices - vector of int timestamps of the same size as the input slice
     * @param dataset - the input timeseries, vector of samples (triplets)
     * @return - estimation for samples before 150 last ones,
     * vector of tuples, where 1st value is index of the sample, 2nd is nonzero estimated switch encoded as
     * -1 - On-Off switch, 1 - Off-On switch. Zero (non-switch) samples are omitted.
     * If buffer size is not enough for estimation, the output is empty of of less size than input
     */
    std::vector<std::tuple<unsigned long long int, value_type>> encode(
            const std::vector<unsigned long long int> & indices,
            const std::vector<std::vector<value_type>> & dataset
            );

    /**
     * @brief make_pairs - converts output of .encode_raw to result similar to output of .encode
     * i.e. removes zero (non-switch) outputs and rearranges data by pairs
     * @param indices - indices of input estimations
     * @param raw_switches - estomated switch values
     * encoded as: 0 - no switch. -1 - On-Off switch, 1 - Off-On switch
     * @return
     */
    std::vector<std::tuple<unsigned long long int, value_type>> make_pairs(
            const std::vector<unsigned long long int> & indices,
            const std::vector<value_type> & raw_switches
            );

    /**
     * @brief saves the model into the Blaze image
     * @param desired filename, file will be overwritten if exists
     */
    void save(const std::string & filename);


private:

    metric::ESN<std::vector<value_type>, void> esn;
    size_t wnd_size; // size of the window used to compute additional window stddev feature
    // and postprocessing entropy filter, currently hardcoded to 15
    size_t cmp_wnd_sz; // size of window used to ensure detected switch really turns state, currently hardcoded to 150
    size_t washout;  // amount of samples to be excluded from training and prediction due to reservoir washout, currently 2500
    value_type contrast_threshold; // rate of contrast between averages in the cmp_wnd_size to the left and to the right,
    // needed to consider that current point contains a switch
    value_type alpha; // ESN metaparameter
    value_type beta; // ESN metaparameter
    std::vector<std::vector<value_type>> buffer = {};  // buffer for accumulation of samples passed online
    std::vector<unsigned long long int> buffer_idx = {};  // indices
    size_t online_cnt = 0; // online sample counter

    /**
     * @brief init - initializes parameters
     */
    //void init();

    /**
     * @brief v_stddev - compute standard deviation of the vector v
     * @param v
     * @param population - flag, determines wheter to compute
     * biased ("population", divided by N) or unbuased ("sample", divide by N-1) standard deviation
     * @return standard deviation
     */
    //value_type v_stddev(const std::vector<value_type> & v);
    value_type v_stddev(const std::vector<value_type> & v, const bool population = false);

    /**
     * @brief preprocess - adds sliding window wtddev feature to the input dataset given as Blaze matrix
     * @param input - dataset passed by user
     * @return - augmented dataset
     */
    blaze::DynamicMatrix<value_type, blaze::rowMajor> preprocess(const blaze::DynamicMatrix<value_type, blaze::rowMajor> & input);

    /**
     * @brief preprocess - adds sliding window wtddev feature to the input dataset given as vector of RecType samples
     * @param input - dataset passed by user
     * @return - augmented dataset
     */
    template <typename RecType>
    std::vector<RecType> preprocess(const std::vector<RecType> & input);

    /**
     * @brief class_entropy - computes entropy of class (switch of no switch) values within sliding window of size wnd_size
     * @param data - vector of class values
     * @param threshold - bound that separate classes, typically 0.5
     * @return entropy value
     */
    value_type class_entropy(const blaze::DynamicVector<value_type> & data, const value_type threshold);

    /**
     * @brief train - trains model using dataset and labels passed as Blaze matrices
     * @param training_data
     * @param labels
     */
    void train(const blaze::DynamicMatrix<value_type> & training_data, const blaze::DynamicMatrix<value_type> & labels);

    /**
     * @brief train - trains model using dataset and labels passed as vectors of samples
     * @param training_data
     * @param labels
     */
    template <typename RecType>  // to be deduced
    void train(const std::vector<RecType> & training_data, const std::vector<RecType> & labels);

};



#include "esn_switch_detector.cpp"

#endif // _METRIC_ESN_SWITCH_DETECTOR_HPP
