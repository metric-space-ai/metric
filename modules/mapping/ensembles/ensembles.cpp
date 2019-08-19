/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_MAPPING_ENSEMBLES_ENSEMBLES_CPP
#define _METRIC_MAPPING_ENSEMBLES_ENSEMBLES_CPP
#include "../ensembles.hpp"
#include <variant>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <memory>
#include <limits>
#include <tuple>

#include <iostream>

#include "DT/edm_wrappers.hpp"

namespace metric {

// Subsample
template <class Record>
template <typename ConType>
void Subsample<Record>::operator()(const ConType& data,  // input dataset
    const std::vector<std::function<double(Record)>>&
        features,  // accessors // no need in features, used as placeholder for possible new overrides
    const std::function<bool(Record)>& response,  // accessors // here response is not used, but it is in overrides
    const std::vector<double>& weights,  // probability that the record gets to the subsample
    double portion,  // size of subsample / overall
    double portion_major,  // placeholder for overrides, not in use in base class
    ConType& out_data,  // output subsample
    bool replacement  // mode of subsampling: with replacement (bootstrap) or without
    ) const
{
    std::vector<El> roulette;

    double sum = 0;
    for (size_t i = 0; i < weights.size(); i++)
        sum += weights[i];

    double intv = 0;
    for (size_t i = 0; i < weights.size(); i++) {
        intv += weights[i];
        El el;
        el.original_idx = i;
        el.value = intv / sum;
        roulette.push_back(el);
    }

    if (portion < 0) {
        portion = 0;
        std::cout << "\nWarning: attempt to specify portion less than 0, set to 0\n";
    }
    if (portion > 1) {
        portion = 1;
        std::cout << "\nWarning: attempt to specify portion greater than 1, set to 1\n";
    }
    int out_size = std::floor(data.size() * portion);
    for (int out_idx = 0; out_idx < out_size; out_idx++) {
        int i = montecarlo(roulette, replacement);
        if (i < 0) {
            std::cout << "\nWarning: unable to get unreplaced sample: no one left in dataset\n";
            break;
        }
        out_data.push_back(data[i]);
    }
}

template <class Record>
int Subsample<Record>::montecarlo(std::vector<El>& roulette, bool replacement) const
{
    double r = (double)std::rand() / (RAND_MAX);
    size_t rSize = roulette.size();
    size_t i;
    for (i = 0; i < rSize; i++) {
        if (r <= roulette[i].value) {
            int ret = roulette[i].original_idx;
            if (replacement) {
                roulette.erase(roulette.begin() + i);
            }
            return ret;
        }
    }
    return -1;
}

template <class Record>
void Subsample<Record>::print_roulette(std::vector<El>& roulette) const  // prints roulette, for debug purpose only
{
    std::cout << std::endl << "roulette:" << std::endl;
    for (int i = 0; i < roulette.size(); i++) {
        std::cout << "value: " << roulette[i].value << ", orig idx: " << roulette[i].original_idx << std::endl;
    }
    std::cout << std::endl;
}

// SubsampleRUS
template <class Record>
template <typename ConType>
void SubsampleRUS<Record>::operator()(const ConType& data,
    const std::vector<std::function<double(Record)>>&
        features,  // accessors // not in use here, but is a part of interface
    const std::function<bool(Record)>& response,  // accessors // here response is not used, but it is in overrides
    const std::vector<double>& weights,  // probability that the record gets to the subsample
    double portion,  // size of subsample / overall
    double portion_major,  // share of major class
    ConType& out_data,  // output subsample
    bool replacement  // mode of subsampling: with replacement (bootstrap) or without
)
{
    if (data.size() > 0)
        if (data[0].size() < 1)  // special check if there is the field in dataset behind the accessor
        {
            out_data = {};
            return;
        }
    double sum_class1 = 0;
    double sum_class0 = 0;
    int count_class1 = 0;
    int count_class0 = 0;
    for (size_t i = 0; i < weights.size(); i++)  // determine major class, find sum of weights for each class
        if (response(data[i])) {
            sum_class1 += weights[i];
            count_class1++;
        } else {
            sum_class0 += weights[i];
            count_class0++;
        }

    bool major_class;
    double sum_major;
    double sum_minor;
    if (count_class0 > count_class1) {
        major_class = false;
        sum_major = sum_class0;
        sum_minor = sum_class1;
    } else {
        major_class = true;
        sum_major = sum_class1;
        sum_minor = sum_class0;
    }

    typedef typename Subsample<Record>::El El;  // defined in base class template
    std::vector<El> roulette_major;  // independent roulettes for each class
    std::vector<El> roulette_minor;

    double intv_major = 0;
    double intv_minor = 0;
    for (size_t i = 0; i < weights.size(); i++)  // fullfill both roulettes in one loop of records
    {
        El el;
        el.original_idx = i;
        if (response(data[i]) == major_class)  // add element to major class roulette
        {
            intv_major += weights[i];  // increment the sector in roulette
            el.value = intv_major / sum_major;
            roulette_major.push_back(el);
        } else  // ... to minor class (assuming we have just 2 classes)
        {
            intv_minor += weights[i];
            el.value = intv_minor / sum_minor;
            roulette_minor.push_back(el);
        }
    }  // roulettes ready

    if (portion < 0) {
        // Warning: attempt to specify portion less than 0, set to 0
        portion = 0;
    }
    if (portion > 1) {
        //Warning: attempt to specify portion greater than 1, set to 1
        portion = 1;
    }
    if (portion_major <= 0) {
        //Warning: attempt to specify portion for major class less than 0, set to 0.5
        portion_major = 0.5;
    }
    if (portion_major >= 1) {
        //Warning: attempt to specify portion for major class greater than 1, set to 0.5
        portion_major = 0.5;
    }

    int out_size = std::floor(data.size() * portion);  // size of output subsample set
    int out_size_major = std::floor(data.size() * portion * portion_major);  // number of major class elements
    int out_size_minor = out_size - out_size_major;
    if (out_size_minor < 0)
        out_size_minor = 0;

    for (int out_idx = 0; out_idx < out_size_major; out_idx++)  // pick the required number of major class elements
    {
        int i = this->montecarlo(roulette_major, replacement);
        if (i < 0) {
            break;
        }
        out_data.push_back(data[i]);
    }
    for (int out_idx = 0; out_idx < out_size_minor; out_idx++)  // pick the required number of minor class elements
    {
        int i = this->montecarlo(roulette_minor, replacement);
        if (i < 0) {
            break;
        }
        out_data.push_back(data[i]);
    }

    std::random_device rng;
    std::mt19937 urng(rng());

    std::shuffle(out_data.begin(), out_data.end(), urng);
}

// TestCl - example learner class
template <class Record>
TestCl<Record>::TestCl(int var_idx_, bool invert_)
    : var_idx(var_idx_)
    , invert(invert_)
{
}

template <class Record>
template <typename ConType>
void TestCl<Record>::train(
    ConType& data, std::vector<std::function<double(Record)>>& features, std::function<bool(Record)>& response)
{  // dummy code, amed to test access to data
    auto max_val = -std::numeric_limits<double>::max();
    auto min_val = std::numeric_limits<double>::max();
    for (auto i = data.cbegin(); i != data.cend(); i++) {
        double current_val = features[var_idx](*i);
        if (current_val < min_val)
            min_val = current_val;
        if (current_val > max_val)
            max_val = current_val;
    }
    middle = min_val + (max_val - min_val) / 2;
    std::cout << "TestCl trained on: " << std::endl;
    for (auto i = data.cbegin(); i != data.cend(); i++)
        std::cout << features[var_idx](*i) << std::endl;
    std::cout << " - foung middle: " << middle << std::endl;
}

template <class Record>
template <typename ConType>
void TestCl<Record>::predict(
    ConType data, std::vector<std::function<double(Record)>> features, std::vector<bool>& predictions)
{  // dummy code, uses only one feature for prediction
    predictions = {};
    for (auto i = data.cbegin(); i != data.cend(); i++)
        if (features[var_idx](*i) >= middle)  // TODO add invert flag usage
            predictions.push_back(true);
        else
            predictions.push_back(false);
}

template <class Record>
std::shared_ptr<metric::TestCl<Record>> TestCl<Record>::clone()
{
    std::shared_ptr<TestCl<Record>> sptr = std::make_shared<TestCl<Record>>(var_idx, invert);
    return sptr;
}  // non-learned classifier factory, meant for ensemble building within strong classifier train method

// Boosting
template <class Record, class WeakLearner,
    typename Subsampler>  // data record type, single weak learner type, subsampler functor type
Boosting<Record, WeakLearner, Subsampler>::Boosting(
    int ensemble_size_, double share_overall_, double share_minor_, WeakLearner weak_classifier_)
    : ensemble_size(ensemble_size_)
    ,  // number of learners to be created
    share_overall(share_overall_)
    ,  // passed to subsampler
    share_minor(share_minor_)
    ,  // passed to subsampler
    weak_classifier(
        weak_classifier_)  // classifier object of type WeakLearner, created by user with desired metaparameters. It will be cloned multiple (ensemble_size_) times into an ensemble, and each instance will be learned using the unique subsample
{
    if (ensemble_size_ < 2) {
        //Warning: specified ensemble size is less than 2, set to minimal possible size 2
        ensemble_size = 2;
    }
    if (share_overall_ <= 0 || share_overall_ >= 1) {
        //Warning: specified wrong overall share value, set to 0.5
        share_overall = 0.5;
    }
    if (share_minor_ <= 0 || share_overall_ >= 1) {
        //Warning: specified wrong minor class share value, set to 0.5
        share_minor = 0.5;
    }
}

template <class Record, class WeakLearner,
    typename Subsampler>  // data record type, single weak learner type, subsampler functor type
template <typename ConType>  // container type
void Boosting<Record, WeakLearner, Subsampler>::train(
    ConType& data,  // labeled data. .size() method is used to determine the size of dataset
    std::vector<std::function<double(Record)>>&
        features,  // feature accessors, .size() method is used  in order to determine number of features
    std::function<bool(Record)>& response,  // label accessors
    bool replacement  // passed to subsampler
)
{
    n_samples = data.size();
    n_features = features.size();
    std::vector<double> D_t;  // sample weights
    double init_val = 1.0 / n_samples;  // initial weights
    for (int i = 0; i < n_samples; i++) {
        D_t.push_back(init_val);  // initial distribution
    }

    ens_alpha.clear();
    ens_classifiers.clear();
    auto new_ensemble_size
        = ensemble_size;  // save desired size, since it can be decreased if subsampler fails to obtain enough subsamples
    for (size_t t = 1; t <= ensemble_size; t++)  //
    {
        ConType subset = {};
        Subsampler subsampler;
        subsampler(
            data, features, response, D_t, share_overall, 1 - share_minor, subset, replacement);  // generate subset
        if (subset.size() < 1) {
            new_ensemble_size--;
            continue;
        }

        auto wcl = weak_classifier.clone();
        wcl->train(subset, features, response);

        std::vector<bool> predictions;
        double eps = 0;
        wcl->predict(data, features, predictions);
        for (int i = 0; i < n_samples; i++) {
            if (predictions[i] != response(data[i]))  // (pred != response(data[i]))
                eps += D_t[i];
        }
        eps = eps / n_samples;  // normalize
        // in some pseudocodes here we check eps >= 0.5, but as we multiply by alpha, that can be negative for inverted classifiers, we can use even inverted classifiers (with another inversion applied)
        double alpha = std::log((1 - eps) / eps) / 2;
        ens_alpha.push_back(alpha);
        ens_classifiers.push_back(wcl);

        double D_t_sum = 0;
        for (int i = 0; i < n_samples; i++) {
            D_t[i] = D_t[i] * exp(-alpha * (response(data[i]) ? 1 : -1) * (predictions[i] ? 1 : -1));
            D_t_sum += D_t[i];
        }

        if (D_t_sum
            != 0)  // TODO profile and (if needed) replace equality check for double with check if all alphas are inf
            for (int i = 0; i < n_samples; i++)  // update weights
            {
                D_t[i] = D_t[i] / D_t_sum;
            }
        else  // special case: weights are undefined
        {
            for (int i = 0; i < n_samples; i++)
                D_t[i] = init_val;
        }
    }
    ensemble_size = new_ensemble_size;
}

template <class Record, class WeakLearner,
    typename Subsampler>  // data record type, single weak learner class, subsampler functor
template <typename ConType>
void Boosting<Record, WeakLearner, Subsampler>::predict(
    ConType& data,  // dataset of unlabeled of labeled data for prediction. Labels will not be accessed
    std::vector<std::function<double(Record)>>& features,  // featuure accessors
    std::vector<bool>& predictions)  // out var
{
    if (features.size() <= 0 || data.size() <= 0) {
        for (size_t i = 0; i < predictions.size(); i++)  // undefined prediction: random output
            predictions[i] = std::rand() % 2 == 1;
        return;
    }
    predictions = std::vector<bool>(data.size(), false);
    auto pred = std::vector<double>(data.size(), 0);
    for (size_t t = 0; t < ensemble_size; t++) {
        std::vector<bool> pr_bool;
        ens_classifiers[t]->predict(data, features, pr_bool);
        for (size_t i = 0; i < data.size(); i++) {
            pred[i] += (pr_bool[i] ? 1 : -1) * ens_alpha[t];
        }
    }
    for (size_t i = 0; i < data.size(); i++)
        predictions[i] = pred[i] >= 0 ? true : false;
}

template <class Record, class WeakLearner,
    typename Subsampler>  // data record type, single weak learner class, subsampler functor
std::shared_ptr<Boosting<Record, WeakLearner, Subsampler>> Boosting<Record, WeakLearner, Subsampler>::clone()
{
    auto weak_sptr = weak_classifier.clone();
    std::shared_ptr<Boosting<Record, WeakLearner, Subsampler>> sptr
        = std::make_shared<Boosting>(ensemble_size, share_overall, share_minor, *weak_sptr);
    return sptr;
}

// Bagging_tuple - old interface based passing weak learner types within a tuple
template <class Record, class WeakLearnerTuple, typename Subsampler>
Bagging_tuple<Record, WeakLearnerTuple, Subsampler>::Bagging_tuple(int ensemble_size_, double share_overall_,
    double share_minor_, std::vector<double> type_weinght_, WeakLearnerTuple weak_classifiers_)
    : ensemble_size(ensemble_size_)
    , share_overall(share_overall_)
    , share_minor(share_minor_)
    , type_weight(type_weinght_)
    , weak_classifiers(weak_classifiers_)
{
}

template <class Record, class WeakLearnerTuple, typename Subsampler>
template <typename ConType>
void Bagging_tuple<Record, WeakLearnerTuple, Subsampler>::train(ConType& data,
    std::vector<std::function<double(Record)>>& features, std::function<bool(Record)>& response, bool replacement)
{
    n_samples = data.size();
    n_features = features.size();
    std::vector<double> D_t;  // sample weights, vector is not needed here, left for compatibility
    double init_val = 1.0 / n_samples;  // initial weights
    for (int i = 0; i < n_samples; i++) {
        D_t.push_back(init_val);
    }
    ensemble = {};  // init tuple of vectors
    double denom = 0;
    for (double s : type_weight)
        denom += s;
    for (int type = 0; type < weak_type_num; type++) {
        for (int t = 1; t <= std::round(ensemble_size * type_weight[type] / denom); t++) {
            ConType subset = {};
            Subsampler subsampler;
            subsampler(data, features, response, D_t, share_overall, 1 - share_minor, subset, replacement);
            // subset ready for training

            std::vector<double> dummy = {};
            for_index(type, weak_classifiers, ensemble, 1, subset, features, response, dummy);
        }
    }
}

template <class Record, class WeakLearnerTuple, typename Subsampler>
template <typename ConType>
void Bagging_tuple<Record, WeakLearnerTuple, Subsampler>::predict(
    ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions)
{
    if (features.size() <= 0 || data.size() <= 0) {
        for (size_t i = 0; i < predictions.size(); i++)  // undefined prediction: random output
            predictions[i] = std::rand() % 2 == 1;
        return;
    }
    predictions = std::vector<bool>(data.size(), false);
    auto pred = std::vector<double>(data.size(), 0);
    for (int type = 0; type < weak_type_num; type++) {
        auto type_prediction = std::vector<double>(data.size(), 0);
        std::function<bool(Record)> dummy_response_var;
        for_index(type, weak_classifiers, ensemble, 2, data, features, dummy_response_var,
            type_prediction);  // TODO consider replace vector<double> with vector<int>
        for (size_t i = 0; i < data.size(); i++)
            pred[i] = pred[i] + type_prediction[i];
    }
    for (size_t i = 0; i < data.size(); i++)
        predictions[i] = pred[i] > 0 ? true : false;
}

// TODO implement clone() if needed to use as weak

template <class Record, class WeakLearnerTuple, typename Subsampler>
template <typename T1, typename T2, typename ConType>
void Bagging_tuple<Record, WeakLearnerTuple,
    Subsampler>::myfunc(  // used both for train and predict in order to access weak learner of any type
    T1& t1, T2& t2, int sw,  // sw is train or predict mode switcher
    ConType& data, std::vector<std::function<double(Record)>>& features, std::function<bool(Record)>& response,
    std::vector<double>& prediction)  // out

{

    if (sw == 1) {  // train // TODO divide myfunc into separate functions, or at least enum and case switcher
        auto wcl = t1.clone();
        wcl->train(data, features, response);
        t2.push_back(wcl);
    } else {
        if (sw == 2) {  // predict for the given type of learners
            prediction = std::vector<double>(data.size(), 0);
            for (size_t t = 0; t < t2.size(); t++) {
                std::vector<bool> pr_bool;
                t2[t]->predict(data, features, pr_bool);
                for (size_t i = 0; i < data.size(); i++) {
                    prediction[i] += (pr_bool[i] ? 1 : -1);
                }
            }
        }
    }
};

// // tuple runtime processing members

template <class Record, class WeakLearnerTuple, typename Subsampler>
template <std::size_t I, typename ConType, typename... Tp, typename... Tp2>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
Bagging_tuple<Record, WeakLearnerTuple, Subsampler>::for_index(int, std::tuple<Tp...>&, std::tuple<Tp2...>&, int,
    ConType&, std::vector<std::function<double(Record)>>&, std::function<bool(Record)>&, std::vector<double>&)
{
}

template <class Record, class WeakLearnerTuple, typename Subsampler>
    template <std::size_t I, typename ConType, typename... Tp, typename... Tp2>
    inline typename std::enable_if
    < I<sizeof...(Tp), void>::type Bagging_tuple<Record, WeakLearnerTuple, Subsampler>::for_index(int index,
        std::tuple<Tp...>& t1, std::tuple<Tp2...>& t2,
        int sw,  // 1 - train, 2 - predict
        ConType& data, std::vector<std::function<double(Record)>>& features, std::function<bool(Record)>& response,
        std::vector<double>& prediction)  // out, only used in prediction mode (sw == 2)
// define for_index function of type void only if I < els in pack
{
    if (index == 0)
        myfunc<el<I, std::tuple<Tp...>>, el<I, std::tuple<Tp2...>>>(
            std::get<I>(t1), std::get<I>(t2), sw, data, features, response, prediction);
    for_index<I + 1>(index - 1, t1, t2, sw, data, features, response, prediction);  //  Tp... not needed
}

//*/

// Bagging - uses variant type for passing weak learners of random type
template <class Record, class WeakLearnerVariant, typename Subsampler>  // vector of variant of weak learners
Bagging<Record, WeakLearnerVariant, Subsampler>::Bagging(int ensemble_size_, double share_overall_, double share_minor_,
    std::vector<double> type_weinght_, std::vector<WeakLearnerVariant> weak_classifiers_)
    : ensemble_size(ensemble_size_)
    ,  // overall number of learners to be created
    share_overall(share_overall_)
    ,  // passed to subsampler
    share_minor(share_minor_)
    ,  // passed to subsampler
    type_weight(type_weinght_)
    ,  // share of learner of each type in ensemble
    weak_classifiers(weak_classifiers_)  // vector of weak classifiers of respective types
{
}

template <class Record, class WeakLearnerVariant, typename Subsampler>
template <typename ConType>
void Bagging<Record, WeakLearnerVariant, Subsampler>::train(
    ConType& data,  // labeled data. .size() method is used to determine the size of dataset
    std::vector<std::function<double(Record)>>&
        features,  // feature accessors, .size() method is used  in order to determine number of features
    std::function<bool(Record)>& response,  // label accessors
    bool replacement  // passed to subsampler
)
{
    auto train_visitor = [&](auto& t) { t.train(data, features, response); };
    n_samples = data.size();
    n_features = features.size();
    std::vector<double> D_t;  // sample weights, used for compatibility ever with initial values
    double init_val = 1.0 / n_samples;
    for (int i = 0; i < n_samples; i++) {
        D_t.push_back(init_val);
    }
    ensemble = {};
    double denom = 0;
    for (double s : type_weight)
        denom += s;
    for (size_t i = 0; i < weak_classifiers.size(); i++)  // learner type loop
    {
        auto learner = weak_classifiers[i];  // copy
        for (int t = 1; t <= std::round(ensemble_size * type_weight[i] / denom);
             t++)  // create and learn the needed number of learners of the current type
        {
            ConType subset = {};
            Subsampler subsampler;
            subsampler(data, features, response, D_t, share_overall, 1 - share_minor, subset, replacement);
            // subset ready for training
            std::visit(train_visitor, learner);
            ensemble.push_back(learner);
        }
    }
}

template <class Record, class WeakLearnerVariant, typename Subsampler>
template <typename ConType>
void Bagging<Record, WeakLearnerVariant, Subsampler>::predict(
    ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions)
{
    if (features.size() <= 0 || data.size() <= 0) {
        for (size_t i = 0; i < predictions.size(); i++)  // undefined prediction: random output
            predictions[i] = std::rand() % 2 == 1;
        return;
    }
    predictions = std::vector<bool>(data.size(), false);
    auto pred = std::vector<int>(data.size(), 0);
    auto pred_bool = std::vector<bool>(data.size(), 0);
    auto predict_visitor = [&](auto& t) { t.predict(data, features, pred_bool); };
    for (auto learner : ensemble) {
        std::visit(predict_visitor, learner);
        for (size_t i = 0; i < data.size(); i++) {
            pred[i] += pred_bool[i] ? 1 : -1;
        }
    }
    for (size_t i = 0; i < data.size(); i++)
        predictions[i] = pred[i] > 0 ? true : false;
}

template <class Record, class WeakLearnerVariant,
    typename Subsampler>  // data record type, single weak learner class, subsampler functor
std::shared_ptr<Bagging<Record, WeakLearnerVariant, Subsampler>>
Bagging<Record, WeakLearnerVariant, Subsampler>::clone()
{
    std::vector<WeakLearnerVariant> clSet = {};
    for (auto learner : weak_classifiers) {
        clSet.push_back(learner);  // no call of clone(), simply copy
    }
    std::shared_ptr<Bagging<Record, WeakLearnerVariant, Subsampler>> sptr
        = std::make_shared<Bagging>(ensemble_size, share_overall, share_minor, type_weight, clSet);
    return sptr;
}

}  // namespace metric
#endif
