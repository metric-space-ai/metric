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
    SwitchPredictor(const blaze::DynamicMatrix<value_type> & training_data, const blaze::DynamicMatrix<value_type> & labels);

    /**
     * @brief create & train model using vectors of STL containers
     *
     * @param training_data - timeseries, each RecType contains 3 values and represents a sample
     * @param labels - vector of containers with single label value in each: -1, 0 or 1.
     * Length should be equal to the length of training_dataset
     */
    template <typename RecType>  // to be deduced
    SwitchPredictor(const std::vector<RecType> & training_data, const std::vector<RecType> & labels);

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
    blaze::DynamicMatrix<value_type> encode_raw(const blaze::DynamicMatrix<value_type> & dataset);

    ///**
    // * @brief estimates switches in offline timeseries
    // *
    // * @param dataset - 3-dimensional timeseries with sample in rows
    // * @return estimation for pre-previous 150 samples. Every 150th call the vector of found switches is returned (if any),
    // * otherwise empty vector.
    // * Switch is represented by tuple: first value is index of a switch found in the processed fragment of length 150,
    // * secong value is class (direction) of the switch (-1 or 1).
    // */
    //std::vector<std::tuple<size_t, value_type>> encode(const blaze::DynamicMatrix<value_type> & dataset);  // TODO implement

    /**
     * @brief estimates switches in offline timeseries
     *
     * @param dataset - vector of 3-dimensional samples
     * @return column of -1, 0 or 1 values, where -1 and 1 represent On-Off and Off-On switches respectively
     */
    template <typename RecType>
    std::vector<value_type> encode_raw(const std::vector<RecType> & dataset);

    ///**
    // * @brief estimates switches in offline timeseries
    // *
    // * @param dataset - vector of 3-dimensional samples
    // * @return estimation for samples. Every 150th call the vector of found switches is returned (if any),
    // * otherwise empty vector.
    // * Switch is represented by tuple: first value is index of a switch found in the processed fragment of length 150,
    // * secong value is class (direction) of the switch (-1 or 1).
    // */
    //template <typename RecType>
    //std::vector<std::tuple<size_t, value_type>> encode(const std::vector<RecType> & dataset);

    /**
     * @brief eestimates switches online. Output describes samples passed by 150 calls made before last 150 calls
     *
     * @param sample - single sample, vector of 3 values
     * @return estimation for pre-previous 150 samples. Every 150th call the vector of found switches is returned (if any),
     * otherwise empty vector.
     * The returned vector is of length 150 filled by  0, -1 or 1 values (no switch, On-Off switch, Off-On switch, respectively
     */
    std::vector<value_type> encode_raw(const std::vector<value_type> & sample, size_t output_size = 0);

    /**
     * @brief estimates switches online. Output describes samples passed by 150 calls made before last 150 calls
     *
     * @param sample - single sample, vector of 3 values
     * @return estimation for pre-previous 150 samples. Every 150th call the vector of found switches is returned (if any),
     * otherwise empty vector.
     * Switch is represented by tuple: first value is index of a switch found in the processed fragment of length 150,
     * secong value is class (direction) of the switch (-1 or 1).
     */
    std::vector<std::tuple<size_t, value_type>> encode(const std::vector<value_type> & sample);

    /**
     * @brief encode_buf - addes the assed slice to buffer and encodes slice of the same size at 150 in the past
     * @param dataset
     * @return estimation for samples before 150 last ones, as column of -1, 0 or 1 values.
     */
    std::vector<value_type> encode_buf_raw(const std::vector<std::vector<value_type>> & dataset);

    /**
     * @brief encode_buf - addes the assed slice to buffer and encodes slice of the same size at 150 in the past
     * @param dataset
     * @return estimation for samples before 150 last ones.
     * Switch is represented by tuple: first value is index of a switch found in the processed fragment of length 150,
     * secong value is class (direction) of the switch (-1 or 1).

     */
    std::vector<std::tuple<size_t, value_type>> encode_buf(const std::vector<std::vector<value_type>> & dataset);

    /**
     * @brief saves the model into the Blaze image
     * @param filename
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
    std::vector<std::vector<value_type>> buffer = {};  // buffer for accumulation of samples passed online
    size_t online_cnt = 0; // online sample counter

    /**
     * @brief init - initializes parameters
     */
    void init();

    /**
     * @brief v_stddev - compute standard deviation of the vector v
     * @param v
     * @return standard deviation
     */
    value_type v_stddev(std::vector<value_type> const & v);

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
    value_type class_entropy(const blaze::DynamicVector<value_type> & data, value_type threshold);

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

    template <typename RecType>
    std::vector<std::tuple<size_t, value_type>> make_indices(const std::vector<RecType> & raw_result);

};



#include "esn_switch_detector.cpp"

#endif // _METRIC_ESN_SWITCH_DETECTOR_HPP
