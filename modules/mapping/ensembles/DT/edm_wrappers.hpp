/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Panda Team
*/

#ifndef _METRIC_MAPPING_ENSEMBLES_DT_EDM_WRAPPERS_HPP
#define _METRIC_MAPPING_ENSEMBLES_DT_EDM_WRAPPERS_HPP
#include <memory>
#include <vector>
#include <functional>

#include "libedm/DataSet.h"
#include "libedm/Classifier.h"
#include "libedm/Prediction.h"
#include "libedm/svm.h"
#include "libedm/b-svm.h"
#include "libedm/C45.h"
#include "libedm/RandSequence.h"

namespace metric {

/**
* @class edmClassifier
*
* @brief
*
*/
template <class Record, class edmCl>
class edmClassifier {
public:
    /**
     * @brief train model on test dataset
     *
     * @param payments test dataset
     * @param features
     * @param response
     */
    template <typename ConType>
    void train(
        ConType& payments, std::vector<std::function<double(Record)>>& features, std::function<bool(Record)>& response);

    /**
    * @brief use model to classify input data
    *
    * @param data input data
    * @param features
    * @param predictions[out] prediction result
    */
    template <typename ConType>
    void predict(ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions);

    /**
     * @brief clone object
     *
     */
    std::shared_ptr<edmClassifier<Record, edmCl>> clone();

protected:
    template <typename ConType>
    void get_prediction(ConType& data, std::vector<std::function<double(Record)>>& features,
        std::shared_ptr<edmCl> model, std::vector<bool>& predictions);

    template <typename ConType>
    libedm::CDataset read_data(ConType& payments, std::vector<std::function<double(Record)>>& features,
        std::function<bool(Record)> response = nullptr) const;

private:
    // ptr is needed due to "named ctor idiom" used in libEDM: default ctor is private
    std::shared_ptr<edmCl> model = nullptr;
};

/**
 * @class edmC45
 * @brief
 *
 */
template <class Record>
class edmC45 : public edmClassifier<Record, libedm::CC45> {
public:
    // ctor takes params for edmCl and stores it in order to use later in Train
    /**
     * @brief Construct a new edm C45 object
     *
     * @param UMINOBJS_
     * @param UEpsilon_
     * @param UCF_
     * @param WillPrune_
     */
    edmC45(int UMINOBJS_ = 2, double UEpsilon_ = 1e-3, double UCF_ = 0.25, double WillPrune_ = true);

    /**
     * @brief train model on test dataset
     *
     * @param payments test dataset
     * @param features
     * @param response
     */
    template <typename ConType>
    void train(
        ConType payments, std::vector<std::function<double(Record)>> features, std::function<bool(Record)> response);

    /**
     * @brief use model to classify input data
     *
     * @param data input data
     * @param features
     * @param predictions[out] prediction result
     */

    template <typename ConType>
    void predict(ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions);

    /**
     * @brief clone object
     *
     */
    std::shared_ptr<edmC45<Record>> clone();

private:
    std::shared_ptr<libedm::CC45> model;

    int UMINOBJS;
    double UEpsilon;
    double UCF;
    double WillPrune;
};

/**
 * @class edmSVM
 * @brief
 *
 */
template <class Record>
class edmSVM : public edmClassifier<Record, libedm::CSVM> {
public:
    // ctor takes params for edmCl and stores it in order to use later in Train
    /**
     * @brief Construct a new edmSVM object
     *
     * @param usvm_type
     * @param ukernel_type
     * @param udegree
     * @param ucoef0
     * @param ucache_size
     * @param ueps
     * @param uC
     * @param unr_weight
     * @param uweight_label
     * @param uweight
     * @param unu
     * @param up
     * @param ushrinking
     * @param uprobability
     */
    edmSVM(int usvm_type = C_SVC, int ukernel_type = RBF, int udegree = 3, double ucoef0 = 0, double ucache_size = 100,
        double ueps = 0.001, double uC = 1, int unr_weight = 0, int* uweight_label = NULL, double* uweight = NULL,
        double unu = 0.5, double up = 0.1, int ushrinking = 1, int uprobability = 0);

    /**
     * @brief train model on test dataset
     *
     * @param payments test dataset
     * @param features
     * @param response
     */
    template <typename ConType>
    void train(
        ConType payments, std::vector<std::function<double(Record)>> features, std::function<bool(Record)> response);

    /**
     * @brief use model to classify input data
     *
     * @param data input data
     * @param features
     * @param predictions[out] prediction result
     */
    template <typename ConType>
    void predict(ConType& data, std::vector<std::function<double(Record)>>& features, std::vector<bool>& predictions);

    /**
     * @brief clone object
     *
     */
    std::shared_ptr<edmSVM<Record>> clone();

private:
    std::shared_ptr<libedm::CSVM> model;

    int svm_type;
    int kernel_type;
    int degree;
    double coef0;
    double cache_size;
    double eps;
    double C;
    int nr_weight;
    int* weight_label;
    double* weight;
    double nu;
    double p;
    int shrinking;
    int probability;
};

}  // namespace metric
#include "edm_wrappers.cpp"
#endif
