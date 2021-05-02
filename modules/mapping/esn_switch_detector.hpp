/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2021 Panda Team
*/

#ifndef _METRIC_ESN_SWITCH_DETECTOR_HPP
#define _METRIC_ESN_SWITCH_DETECTOR_HPP


#include "ESN.hpp"



template <typename value_type>
class SwitchPredictor {

public:

    SwitchPredictor(const blaze::DynamicMatrix<value_type> & training_data, const blaze::DynamicMatrix<value_type> & labels);

    template <typename RecType>  // to be deduced
    SwitchPredictor(const std::vector<RecType> & training_data, const std::vector<RecType> & labels);

    SwitchPredictor(const std::string & filename);

    blaze::DynamicMatrix<value_type> encode(const blaze::DynamicMatrix<value_type> & dataset);

    template <typename RecType>
    std::vector<value_type> encode(const std::vector<RecType> & dataset);

    std::vector<value_type> encode_raw(const std::vector<value_type> & sample);

    std::vector<std::tuple<size_t, value_type>> encode(const std::vector<value_type> & sample);

    void save(const std::string & filename);


private:

    metric::ESN<std::vector<value_type>, void> esn;
    size_t wnd_size;
    size_t cmp_wnd_sz;
    size_t washout;
    value_type contrast_threshold;
    std::vector<std::vector<value_type>> buffer = {};
    size_t online_cnt = 0;

    void init();

    value_type v_stddev(std::vector<value_type> const & v);

    blaze::DynamicMatrix<value_type, blaze::rowMajor> preprocess(const blaze::DynamicMatrix<value_type, blaze::rowMajor> & input);

    template <typename RecType>
    std::vector<RecType> preprocess(const std::vector<RecType> & input);

    value_type class_entropy(const blaze::DynamicVector<value_type> & data, value_type threshold);

    void train(const blaze::DynamicMatrix<value_type> & training_data, const blaze::DynamicMatrix<value_type> & labels);

    template <typename RecType>  // to be deduced
    void train(const std::vector<RecType> & training_data, const std::vector<RecType> & labels);

};



#include "esn_switch_detector.cpp"

#endif // _METRIC_ESN_SWITCH_DETECTOR_HPP
