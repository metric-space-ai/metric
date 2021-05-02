/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2021 Panda Team
*/


#include "esn_switch_detector.hpp"


//#include <iostream> // TODO remove



template <typename value_type>
SwitchPredictor<value_type>::SwitchPredictor(const blaze::DynamicMatrix<value_type> & training_data, const blaze::DynamicMatrix<value_type> & labels) {
    init();
    train(training_data, labels);
}

template <typename value_type>
template <typename RecType>  // to be deduced
SwitchPredictor<value_type>::SwitchPredictor(const std::vector<RecType> & training_data, const std::vector<RecType> & labels) {
    init();
    train(training_data, labels);
}

template <typename value_type>
SwitchPredictor<value_type>::SwitchPredictor(const std::string & filename) {
    init();
    esn = metric::ESN<std::vector<value_type>, void>(filename);
}



template <typename value_type>
blaze::DynamicMatrix<value_type>
SwitchPredictor<value_type>::encode(const blaze::DynamicMatrix<value_type> & dataset) {

    auto data = preprocess(dataset);

    auto prediction = esn.predict(data);
    blaze::DynamicMatrix<value_type, blaze::rowMajor> out = blaze::trans(prediction);

    blaze::DynamicMatrix<value_type> sl_entropy (out.rows(), 1, 0);
    for (size_t i = wnd_size; i < out.rows(); ++i) {
        blaze::DynamicMatrix<value_type> wnd_row = blaze::submatrix(out, i - wnd_size, 0, wnd_size, 1);
        blaze::DynamicVector<value_type> wnd = blaze::column(wnd_row, 0); //blaze::trans(blaze::column(wnd_row, 0));
        sl_entropy(i, 0) = class_entropy(wnd, 0.5);
    }

    blaze::DynamicMatrix<value_type> postproc (out.rows(), 1, 0);
    bool prev_l_flag = false;
    for (size_t i = cmp_wnd_sz; i < out.rows() - cmp_wnd_sz; ++i) {
        bool l_flag = false;
        if (sl_entropy(i, 0) > 0.4) {
            blaze::DynamicMatrix<value_type> wnd_past = blaze::submatrix(out, i - cmp_wnd_sz, 0, cmp_wnd_sz, 1);
            blaze::DynamicMatrix<value_type> wnd_fut  = blaze::submatrix(out, i, 0, cmp_wnd_sz, 1);
            int label = 0;
            if (blaze::mean(wnd_past) - blaze::mean(wnd_fut) < -contrast_threshold) {  // TODO determine!!
                label = 1;
                l_flag = true;
            }
            if (blaze::mean(wnd_past) - blaze::mean(wnd_fut) > contrast_threshold) {  // TODO determine!!
                label = -1;
                l_flag = true;
            }
            if (!prev_l_flag)
                postproc(i, 0) = label;
        }
        prev_l_flag = l_flag;
    }

    return postproc;
}

template <typename value_type>
template <typename RecType>
std::vector<value_type>
SwitchPredictor<value_type>::encode(const std::vector<RecType> & dataset) {

    auto data = preprocess(dataset);

    auto out = esn.predict(data);

    std::vector<value_type> sl_entropy (out.size(), 0);
    //sl_entropy.reserve(out.size());
    for (size_t i = wnd_size; i < out.size(); ++i) {
        blaze::DynamicVector<value_type> wnd (wnd_size);
        for (size_t j = 0; j < wnd_size; ++j) {
            wnd[j] = out[i - wnd_size + j][0];  // TODO remove ugly inner containers or update for multidimensional labels
        }
        //std::cout << std::endl << wnd << std::endl; TODO remove
        sl_entropy[i] = class_entropy(wnd, 0.5);
    }

    std::vector<value_type> postproc (out.size(), 0);
    //postproc.reserve(out.size());
    bool prev_l_flag = false;
    for (size_t i = cmp_wnd_sz; i < out.size() - cmp_wnd_sz; ++i) {
        bool l_flag = false;
        if (sl_entropy[i] > 0.4) {
            value_type mean_past = 0;
            value_type mean_fut = 0;
            for (size_t j = 0; j < cmp_wnd_sz; ++j) {
                mean_past += out[i - cmp_wnd_sz + j][0];
                mean_fut += out[i + j][0];
            }
            mean_past /= (value_type)cmp_wnd_sz;
            mean_fut /= (value_type)cmp_wnd_sz;
            int label = 0;
            if (mean_past - mean_fut < -contrast_threshold) {
                label = 1;
                l_flag = true;
            }
            if (mean_past - mean_fut > contrast_threshold) {  // TODO determine!!
                label = -1;
                l_flag = true;
            }
            if (!prev_l_flag)
                postproc[i] = label;
        }
        prev_l_flag = l_flag;
    }

    return postproc;
}


template <typename value_type>
std::vector<value_type>
SwitchPredictor<value_type>::encode_raw(const std::vector<value_type> & sample) {

    std::vector<value_type> result = {}; // TODO update

    buffer.push_back(sample);
    if (buffer.size() >= washout + 3*cmp_wnd_sz) { // warmup finished
        if (online_cnt < cmp_wnd_sz - 1) { // no estimation
            ++online_cnt;
        } else { // estimation
            online_cnt = 0;
            std::vector<value_type> all_result = encode(buffer);
            result.insert(result.begin(), all_result.end() - 2*cmp_wnd_sz, all_result.end() - cmp_wnd_sz);
        }
        buffer.erase(buffer.begin()); // pop_front
    }

    return result;
}


template <typename value_type>
std::vector<std::tuple<size_t, value_type>>
SwitchPredictor<value_type>::encode(const std::vector<value_type> & sample) {

    auto raw_result = encode_raw(sample);

    std::vector<std::tuple<size_t, value_type>> result = {};
    for (size_t i = 0; i < raw_result.size(); ++i) {
        if (raw_result[i] != 0) {
            auto sw = std::make_tuple(i, raw_result[i]);
            result.push_back(sw);
        }
    }
    return result;
}


template <typename value_type>
void
SwitchPredictor<value_type>::save(const std::string & filename) {

    esn.save(filename);
}


template <typename value_type>
value_type
SwitchPredictor<value_type>::v_stddev(std::vector<value_type> const & v) {
    value_type mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    value_type sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0,
        [](value_type const & x, value_type const & y) { return x + y; },
        [mean](value_type const & x, value_type const & y) { return (x - mean)*(y - mean); });
    return std::sqrt(sq_sum / v.size());
}


template <typename value_type>
blaze::DynamicMatrix<value_type, blaze::rowMajor>
SwitchPredictor<value_type>::preprocess(const blaze::DynamicMatrix<value_type, blaze::rowMajor> & input) {

    blaze::DynamicVector<value_type> feature_stddev (input.rows(), 0);
    int new_label = 0;
    for (size_t i = wnd_size; i < feature_stddev.size(); ++i) {
        auto wnd1 = blaze::submatrix(input, i - wnd_size, 0, wnd_size, 1);
        auto wnd2 = blaze::submatrix(input, i - wnd_size, 1, wnd_size, 1);
        auto wnd3 = blaze::submatrix(input, i - wnd_size, 2, wnd_size, 1);
        feature_stddev[i] = stddev(wnd1) + stddev(wnd2) + stddev(wnd3);
    }

    blaze::DynamicMatrix<value_type> ds_all (input.rows(), 4, 0);
    blaze::submatrix(ds_all, 0, 0, input.rows(), 3) = blaze::submatrix(input, 0, 0, input.rows(), 3);
    blaze::column(ds_all, 3) = feature_stddev;
    blaze::DynamicMatrix<value_type, blaze::rowMajor> output = blaze::trans(ds_all);

    return output;
}


template <typename value_type>
template <typename RecType>
std::vector<RecType>
SwitchPredictor<value_type>::preprocess(const std::vector<RecType> & input) {

    int new_label = 0;

    std::vector<RecType> output (input);
    for (size_t i = 0; i < output.size(); ++i) {
        if (i < wnd_size) {
            output[i].push_back(0);
        } else {
            std::vector<value_type> wnd1 (wnd_size, 0);
            std::vector<value_type> wnd2 (wnd_size, 0);
            std::vector<value_type> wnd3 (wnd_size, 0);
            for (size_t j = 0; j < wnd_size; ++j) {
                wnd1[j] = output[i - j][0];
                wnd2[j] = output[i - j][1];
                wnd3[j] = output[i - j][2];
            }
            output[i].push_back(v_stddev(wnd1) + v_stddev(wnd2) + v_stddev(wnd3));
            //auto n = output[i];  // TODO remove
        }
    }
    //auto nn = output[0];  // TODO remove
    return output;
}


template <typename value_type>
value_type
SwitchPredictor<value_type>::class_entropy(const blaze::DynamicVector<value_type> & data, value_type threshold) {
    int sum = 0;
    value_type sz = data.size();
    for (size_t i = 0; i<sz; ++i) {
        if (data[i] > threshold)
            ++sum;
    }
    value_type p1 = sum/sz;
    if (sum == 0 || sum == sz)
        return 0;
    else
        return -p1*log2(p1) - (1 - p1)*log2(1 - p1);
}


template <typename value_type>
void
SwitchPredictor<value_type>::init() {  // TODO get parameters from user
    wnd_size = 15;
    cmp_wnd_sz = 150;
    contrast_threshold = 0.3;
    washout = 2500;  // TODO update depending on loaded model
}


template <typename value_type>
void
SwitchPredictor<value_type>::train(const blaze::DynamicMatrix<value_type> & training_data, const blaze::DynamicMatrix<value_type> & labels) {

    assert(training_data.rows() == labels.rows());
    assert(training_data.columns() == 3);  // TODO relax

    auto data = preprocess(training_data);

    blaze::DynamicMatrix<value_type> target (labels.rows(), 1, 0);
    int new_label = 0;
    for (size_t i = wnd_size; i < labels.rows(); ++i) {
        if (labels(i, 0) >= 1)
            new_label = 1;
        if (labels(i, 0) <= -1)
            new_label = 0;
        target(i, 0) = new_label;
    }

    esn = metric::ESN<std::vector<value_type>, void>(500, 5, 0.99, 0.1, washout, 0.5); // TODO pass
    esn.train(data, blaze::trans(target));
}


template <typename value_type>
template <typename RecType>  // to be deduced
void
SwitchPredictor<value_type>::train(const std::vector<RecType> & training_data, const std::vector<RecType> & labels) {

    assert(training_data.size() == labels.size());
    assert(training_data[0].size() == 3);  // TODO relax

    auto data = preprocess(training_data);

    std::vector<RecType> target (labels.size(), {0});
    int new_label = 0;
    for (size_t i = wnd_size; i < labels.size(); ++i) {
        if (labels[i][0] >= 1)
            new_label = 1;
        if (labels[i][0] <= -1)
            new_label = 0;
        target[i] = {new_label};
    }

    esn = metric::ESN<std::vector<value_type>, void>(500, 5, 0.99, 0.1, washout, 0.5); // TODO pass
    esn.train(data, target);
}

