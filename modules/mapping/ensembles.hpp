/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_ENSEMBLES_HPP
#define _METRIC_MAPPING_ENSEMBLES_HPP

#include "ensembles/DT.hpp"

#include <vector>
#include <functional>
#include <memory>
#include <variant>
#include <limits>
#include <tuple>

namespace metric {

/**
 * @class Subsample
 * @brief base subsampler functor class template
 * 
 */
template <class Record>
class Subsample {
public:
    /**
   * @class El
   * @brief
   * 
   */
    struct El {
        int original_idx;
        double value;
    };

    /**
     * @brief 
     * 
     * @param data 
     * @param features 
     * @param response 
     * @param weights 
     * @param portion 
     * @param portion_major 
     * @param out_data 
     * @param replacement 
     */
    template <typename ConType>
    void operator()(const ConType& data, const std::vector<std::function<double(Record)>>& features,
        const std::function<bool(Record)>& response, const std::vector<double>& weights, double portion,
        double portion_major,  // not in use in base class
        ConType& out_data, bool replacement = false) const;

    /**
     * @brief 
     * 
     * @param roulette 
     * @param replacement 
     * @return
     */
    int montecarlo(std::vector<El>& roulette, bool replacement = false) const;

    /**
     * @brief 
     * 
     * @param roulette 
     */
    void print_roulette(std::vector<El>& roulette) const;
};

/**
 * @class SubsampleRUS
 * @brief 
 * 
 */
template <class Record>
class SubsampleRUS : public Subsample<Record> {
public:
    /**
   * @brief 
   * 
   * @param data 
   * @param features 
   * @param response 
   * @param weights 
   * @param portion 
   * @param portion_major 
   * @param out_data 
   * @param replacement 
   */
    template <typename ConType>
    void operator()(const ConType& data, const std::vector<std::function<double(Record)>>& features,
        const std::function<bool(Record)>& response, const std::vector<double>& weights, double portion,
        double portion_major, ConType& out_data, bool replacement = false);
};

/**
 * @class TestCl
 * @brief 
 * 
 */
template <class Record>
class TestCl {

public:
    /**
     * @brief Construct a new Test Cl object
     * 
     * @param var_idx_ 
     * @param invert_ 
     */
    TestCl(int var_idx_, bool invert_);

    /**
     * @brief train model on test data set
     * 
     * @param data test dataset
     * @param features 
     * @param response 
     */
    template <typename ConType>
    void train(
        ConType& data, std::vector<std::function<double(Record)>>& features, std::function<bool(Record)>& response);

    /**
     * @brief 
     * 
     * @param data 
     * @param features 
     * @param predictions 
     */
    template <typename ConType>
    void predict(ConType data, std::vector<std::function<double(Record)>> features, std::vector<bool>& predictions);

    /**
     * @brief clone object
     * 
     * @return shared pointer to clone of the current object
     */
    std::shared_ptr<TestCl<Record>> clone();

private:
    int var_idx;  // specific input param
    bool invert;  // specific input param
    double middle;  // specific for classifier learned model data
};

/**
 * @class Boosting
 * @brief 
 * 
 * @tparam Record data record type
 * @tparam WeakLearner single weak learner class
 * @tparam Subsampler subsampler functor
 */
template <class Record, class WeakLearner, typename Subsampler>
class Boosting {
public:
    /**
     * @brief Construct a new Boosting object
     * 
     * @param ensemble_size_ 
     * @param share_overall_ 
     * @param share_minor_ 
     * @param weak_classifier_ 
     */
    Boosting(int ensemble_size_, double share_overall_, double share_minor_, WeakLearner weak_classifier_);

    /**
     * @brief train model on test dataset
     * 
     * @param data 
     * @param features 
     * @param response 
     * @param replacement 
     */
    template <typename ConType>
    void train(ConType& data, std::vector<std::function<double(Record)>>& features,
        std::function<bool(Record)>& response, bool replacement = false);

    /**
     * @brief 
     * 
     * @param data 
     * @param features 
     * @param predictions 
     */
    template <typename ConType>
    void predict(ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions);
    /**
     * @brief create clone of the current object
     * 
     * @return shared pointer to clone of the current object
     */
    std::shared_ptr<Boosting<Record, WeakLearner, Subsampler>> clone();

private:
    size_t ensemble_size;
    double share_overall;
    double share_minor;
    WeakLearner weak_classifier;
    int n_samples;
    int n_features;
    std::vector<double> ens_alpha;
    std::vector<std::shared_ptr<WeakLearner>> ens_classifiers;
    std::vector<std::shared_ptr<WeakLearner>>
        cloned_classifiers;  // when building the tree of classifiers // TODO think on the better solution
};

/**
 * @class Bagging_tuple
 * @brief 
 * 
 * @tparam Record data record type
 * @tparam WeakLearnerTuple tuple of weak learners
 * @tparam Subsampler subsampler function
 */
template <class Record, class WeakLearnerTuple, typename Subsampler>  // tuple of weak learners
class Bagging_tuple {
public:
    /**
     * @brief Construct a new Bagging_tuple object
     * 
     * @param ensemble_size_ 
     * @param share_overall_ 
     * @param share_minor_ 
     * @param type_weinght_ 
     * @param weak_classifiers_ 
     */
    Bagging_tuple(int ensemble_size_, double share_overall_, double share_minor_, std::vector<double> type_weinght_,
        WeakLearnerTuple weak_classifiers_);

    /**
     * @brief train model on test dataset
     * 
     * @param data 
     * @param features 
     * @param response 
     * @param replacement 
     */
    template <typename ConType>
    void train(ConType& data, std::vector<std::function<double(Record)>>& features,
        std::function<bool(Record)>& response, bool replacement = false);

    /**
     * @brief 
     * 
     * @tparam ConType 
     * @param data 
     * @param features 
     * @param predictions 
     */
    template <typename ConType>
    void predict(ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions);

    // TODO implement clone() if needed to use as weak

private:
    template <typename T1, typename T2, typename ConType>
    void myfunc(  // TODO refactor if needed
        T1& t1, T2& t2, int sw, ConType& data, std::vector<std::function<double(Record)>>& features,
        std::function<bool(Record)>& response,
        std::vector<double>& prediction);  // out

    // // tuple runtime processing members

    template <size_t I, typename T>
    using el = typename std::tuple_element<I, T>::type;

    template <std::size_t I = 0, typename ConType, typename... Tp, typename... Tp2>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type for_index(int, std::tuple<Tp...>&,
        std::tuple<Tp2...>&, int, ConType&, std::vector<std::function<double(Record)>>&, std::function<bool(Record)>&,
        std::vector<double>&);

    template <std::size_t I = 0, typename ConType, typename... Tp, typename... Tp2>
        inline typename std::enable_if
        < I<sizeof...(Tp), void>::type for_index(int index, std::tuple<Tp...>& t1, std::tuple<Tp2...>& t2,
            int sw,  // 1 - train, 2 - predict
            ConType& data, std::vector<std::function<double(Record)>>& features, std::function<bool(Record)>& response,
            std::vector<double>& prediction);  // out, only used in prediction mode (sw == 2)
        // define for_index function of type void only if I < els in pack

    // // creating tuple of weak learner vectors

    template <size_t I, typename T>
    struct tuple_n {
        template <typename... Args>
        using type = typename tuple_n<I - 1, T>::template type<std::vector<std::shared_ptr<el<I - 1, T>>>, Args...>;
    };  // TODO replace with shared_ptr<el<I-1, T>

    template <typename T>
    struct tuple_n<0, T> {
        template <typename... Args>
        using type = std::tuple<Args...>;
    };

    template <typename T>
    using tuple_of = typename tuple_n<std::tuple_size<T>::value, T>::template type<>;

    int ensemble_size;
    double share_overall;
    double share_minor;
    std::vector<double> type_weight;
    WeakLearnerTuple weak_classifiers;
    tuple_of<WeakLearnerTuple> ensemble;  // recursively defined type
    int n_samples;
    int n_features;
    int const weak_type_num = std::tuple_size<WeakLearnerTuple>::value;
};

/**
 * @class Bagging
 * @brief 
 * 
 * @tparam Record
 * @tparam WeakLearnerVariant 
 * @tparam Subsampler 
 */
template <class Record, class WeakLearnerVariant, typename Subsampler>  // tuple of weak learners
class Bagging {
public:
    /**
     * @brief Construct a new Bagging object
     * 
     * @param ensemble_size_ 
     * @param share_overall_ 
     * @param share_minor_ 
     * @param type_weinght_ 
     * @param weak_classifiers_ 
     */
    Bagging(int ensemble_size_, double share_overall_, double share_minor_, std::vector<double> type_weinght_,
        std::vector<WeakLearnerVariant> weak_classifiers_);

    /**
     * @brief train model on test dataset
     * 
     * @param data 
     * @param features 
     * @param response 
     * @param replacement 
     */
    template <typename ConType>
    void train(ConType& data, std::vector<std::function<double(Record)>>& features,
        std::function<bool(Record)>& response, bool replacement = false);

    /**
     * @brief 
     * 
     * @param data 
     * @param features 
     * @param predictions 
     */
    template <typename ConType>
    void predict(ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions);
    /**
     * @brief create clone of current object
     * 
     * @return shared_pointer to clone of the current object
     */
    std::shared_ptr<Bagging<Record, WeakLearnerVariant, Subsampler>> clone();

private:
    int ensemble_size;
    double share_overall;
    double share_minor;
    std::vector<double> type_weight;
    std::vector<WeakLearnerVariant> weak_classifiers;
    std::vector<WeakLearnerVariant> ensemble;
    int n_samples;
    int n_features;
};

}  // namespace metric

#include "ensembles/ensembles.cpp"

#endif
