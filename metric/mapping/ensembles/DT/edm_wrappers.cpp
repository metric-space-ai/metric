/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2018 Panda Team
*/

#ifndef _METRIC_MAPPING_ENSEMBLES_DT_EDM_WRAPPERS_CPP
#define _METRIC_MAPPING_ENSEMBLES_DT_EDM_WRAPPERS_CPP
#include <exception>
namespace metric {

template <typename Record, typename edmCl>
template <typename ConType>
void edmClassifier<Record, edmCl>::train(ConType &payments, std::vector<std::function<double(Record)>> &features,
										 std::function<bool(Record)> &response)
{
	if (features.size() == 0)
		return;
	CDataset data = this->read_data(payments, features, response);
	if (features.size() > 0)
		model = std::make_shared<edmCl>(data); // in get_prediction we check if model == nullptr and return default
	// undefuned (random) prefiction if so
}
template <typename Record, typename edmCl>
template <typename ConType>
void edmClassifier<Record, edmCl>::predict(ConType &data, std::vector<std::function<double(Record)>> &features,
										   std::vector<bool> &predictions)
{
	get_prediction(data, features, model, predictions);
}

template <typename Record, typename edmCl>
std::shared_ptr<edmClassifier<Record, edmCl>> edmClassifier<Record, edmCl>::clone()
{
	// TODO check if not learned!
	std::shared_ptr<edmClassifier<Record, edmCl>> sptr = std::make_shared<edmClassifier<Record, edmCl>>();
	return sptr;
}
template <typename Record, typename edmCl>
template <typename ConType>
void edmClassifier<Record, edmCl>::get_prediction(ConType &data, std::vector<std::function<double(Record)>> &features,
												  std::shared_ptr<edmCl> model, std::vector<bool> &predictions)
{ // aimed for usage in predict functions of derived classes
	predictions = std::vector<bool>(data.size(), false);
	if (features.size() <= 0 || data.size() <= 0 || model == nullptr) {
		for (size_t i = 0; i < predictions.size(); i++) // undefined prediction: random output
			predictions[i] = std::rand() % 2 == 1;
		return;
	}
	CDataset dsdata = this->read_data(data, features);

	CPrediction *result = model->Classify(dsdata);
	std::vector<int> labels = result->GetPredictedLabelIndices();

	for (size_t i = 0; i < data.size(); i++) {
		if (labels[i] == 0)
			predictions[i] = false;
		else {
			if (labels[i] == 1)
				predictions[i] = true;
			else {
				throw std::runtime_error("ERROR IN EDM PREDICTION");
				predictions[i] = false;
			}
		}
	}
}
template <typename Record, typename edmCl>
template <typename ConType>
libedm::CDataset edmClassifier<Record, edmCl>::read_data(ConType &payments,
														 std::vector<std::function<double(Record)>> &features,
														 std::function<bool(Record)> response) const
{

	std::vector<AttrStr> attributes = {};
	std::vector<bool> min_set = {};
	size_t n_attr;
	if (response != nullptr)
		n_attr = features.size() + 1;
	else
		n_attr = features.size();
	for (size_t j = 0; j < n_attr; j++) {
		AttrStr att_str = {
			// features
			2,									 // int AttType // 2 = continous
			"attr" + std::to_string(j),			 // string Name
			-std::numeric_limits<double>::max(), // double Max
			std::numeric_limits<double>::max(),	 // double Min
			false,								 // bool MMSet // Have max and min value been set?
			(int)j,								 // int OtherPos // position of an attribute
			{} // std::vector<DiscValueStr> Disc // discrete attribute: list of all values
		};
		attributes.push_back(att_str);
		min_set.push_back(false);
	}

	std::vector<DiscValueStr> classes = {};
	if (response == nullptr) {
		DiscValueStr class0;
		class0.Name = "0";
		DiscValueStr class1;
		class1.Name = "1";
		classes = {class0, class1};
		AttrStr att_str = {
			// label
			3,					  // int AttType // 3 = class label
			"label",			  // string Name
			1,					  // double Max // seems to be not in use
			0,					  // double Min
			false,				  // bool MMSet // Have max and min value been set?
			(int)features.size(), // int OtherPos // position of an attribute
			classes				  // std::vector<DiscValueStr> Disc // discrete attribute: list of all values
		};
		attributes.push_back(att_str);
	}

	std::vector<InstanceStr> matrix = {};
	for (auto i = payments.cbegin(); i != payments.cend(); i++) // (size_t i = 0; i < payments.size(); i++)
	{
		std::vector<ValueData> rec = {};
		for (size_t j = 0; j < n_attr; j++) {
			ValueData write_val;
			double read_val;
			if (j < features.size()) {
				read_val = features[j](*i); //(payments[i]);
				write_val.Cont = read_val;
			} else {
				read_val = response(*i) ? 1 : 0; //(payments[i]) ? 1 : 0; // test (int)response
				write_val.Discr = read_val;		 // read_val; // labels are discrete
			}
			rec.push_back(write_val);
			if (read_val < attributes[j].Min) { // find min, max
				attributes[j].Min = read_val;
				min_set[j] = true;
			}
			if (read_val > attributes[j].Max) {
				attributes[j].Max = read_val;
				if (min_set[j])
					attributes[j].MMSet = true;
			}
		}
		matrix.push_back(rec);
	}

	CASE_INFO case_info = {
		(int)n_attr,		  // int ReadWidth // number of attribute in each row(including label)
		(int)n_attr,		  // int ValidWidth;//number of real attribute (ignored attributes are excluded)
		2,					  // int ClassNum
		(int)payments.size(), // int Height;//number of instances
		classes,			  // std::vector<DiscValueStr> Classes
		attributes,			  // std::vector<AttrStr> ReadAttrs;//all attributes in a row (including label)
		attributes // std::vector<AttrStr> ValidAttrs;//all attributes in a row (ignored attributes are excluded)
	};
	return libedm::CDataset(case_info, matrix);
}

template <typename Record> edmC45<Record>::edmC45(int UMINOBJS_, double UEpsilon_, double UCF_, double WillPrune_)
{
	UMINOBJS = UMINOBJS_;
	UEpsilon = UEpsilon_;
	UCF = UCF_;
	WillPrune = WillPrune_;
}

template <class Record>
template <typename ConType>
void edmC45<Record>::train(ConType payments, std::vector<std::function<double(Record)>> features,
						   std::function<bool(Record)> response)
{
	if (features.size() == 0)
		return;
	CDataset data = this->read_data(payments, features, response);
	model = std::make_shared<libedm::CC45>(data, UMINOBJS, UEpsilon, UCF, WillPrune);
}

template <class Record>
template <typename ConType>
void edmC45<Record>::predict(ConType &data, std::vector<std::function<double(Record)>> &features,
							 std::vector<bool> &predictions)
{
	this->get_prediction(data, features, model, predictions);
}

template <class Record> std::shared_ptr<edmC45<Record>> edmC45<Record>::clone()
{
	std::shared_ptr<edmC45<Record>> sptr = std::make_shared<edmC45<Record>>(UMINOBJS, UEpsilon, UCF, WillPrune);
	return sptr;
}

template <typename Record>
edmSVM<Record>::edmSVM(int usvm_type, int ukernel_type, int udegree, double ucoef0, double ucache_size, double ueps,
					   double uC, int unr_weight, int *uweight_label, double *uweight, double unu, double up,
					   int ushrinking, int uprobability)
{
	svm_type = usvm_type;
	kernel_type = ukernel_type;
	degree = udegree;
	coef0 = ucoef0;
	cache_size = ucache_size;
	eps = ueps;
	C = uC;
	nr_weight = unr_weight;
	weight_label = uweight_label;
	weight = uweight;
	nu = unu;
	p = up;
	shrinking = ushrinking;
	probability = uprobability;
}

template <typename Record>
template <typename ConType>
void edmSVM<Record>::train(ConType payments, std::vector<std::function<double(Record)>> features,
						   std::function<bool(Record)> response)
{
	if (features.size() == 0)
		return;
	CDataset data = this->read_data(payments, features, response);
	model = std::make_shared<libedm::CSVM>(data, svm_type, kernel_type, degree, coef0, cache_size, eps, C, nr_weight,
										   weight_label, weight, nu, p, shrinking, probability);
}

template <typename Record>
template <typename ConType>
void edmSVM<Record>::predict(ConType &data, std::vector<std::function<double(Record)>> &features,
							 std::vector<bool> &predictions)
{
	this->get_prediction(data, features, model, predictions);
}

template <typename Record> std::shared_ptr<edmSVM<Record>> edmSVM<Record>::clone()
{
	// TODO check if not learned!
	std::shared_ptr<edmSVM<Record>> sptr =
		std::make_shared<edmSVM<Record>>(svm_type, kernel_type, degree, coef0, cache_size, eps, C, nr_weight,
										 weight_label, weight, nu, p, shrinking, probability);
	return sptr;
}

} // namespace metric
#endif
