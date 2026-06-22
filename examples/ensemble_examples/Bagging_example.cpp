/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

// Demonstrates the METRIC-owned Bagging ensemble over native weak learners.
// (The former libedm-backed C4.5/SVM weak learners were removed together with
// the metric/3rdparty/libedm island; Bagging works with any weak learner that
// implements the train/predict/clone contract -- here mtrc::TestCl.)

#include "metric/mapping/ensembles.hpp"

#include "assets/helpers.cpp" // csv reader

#include <chrono>
#include <deque> // for Record test
#include <functional>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

template <typename T> void vector_print(const std::vector<T> &vec)
{

	std::cout << "[";
	for (size_t i = 0; i < vec.size(); i++) {
		if (i < vec.size() - 1) {
			std::cout << vec[i] << ", ";
		} else {
			std::cout << vec[i] << "]" << std::endl;
		}
	}
}

int main()
{
	std::cout << "Bagging example have started" << std::endl;
	std::cout << '\n';

	// test on Iris

	using IrisRec = std::vector<std::string>;

	std::vector<IrisRec> iris_str = read_csv<std::vector<IrisRec>>("./assets/iris.csv");
	std::deque<IrisRec> iris_strD = read_csv<std::deque<IrisRec>>("./assets/iris.csv");

	iris_str.erase(iris_str.begin());	// remove headers
	iris_strD.erase(iris_strD.begin()); // remove headers

	std::vector<IrisRec> IrisTestRec = {iris_str[5]};  // 1
	std::deque<IrisRec> IrisTestRecD = {iris_strD[5]}; // 1

	std::vector<IrisRec> IrisTestMultipleRec = {iris_str[5], iris_str[8], iris_str[112]};	 // 1, 1, 0
	std::deque<IrisRec> IrisTestMultipleRecD = {iris_strD[5], iris_strD[8], iris_strD[112]}; // 1, 1, 0

	std::cout << iris_str.size() << std::endl;
	std::cout << iris_str[0].size() << std::endl;

	std::vector<std::function<double(IrisRec)>> features_iris;
	for (int i = 1; i < (int)iris_str[0].size() - 1; ++i) { // skip 1st column (index)
		features_iris.push_back([=](auto r) { return std::stod(r[i]); } // we need closure: [=] instead of [&]
		);
	}

	std::function<bool(IrisRec)> response_iris = [](IrisRec r) {
		if (r[r.size() - 1] == "\"setosa\"")
			return true;
		else
			return false;
	};

	for (size_t i = 0; i < iris_str[0].size() - 2; i++)
		std::cout << features_iris[i](iris_str[10]) << ", ";
	std::cout << std::endl;

	std::vector<bool> prediction;

	// Bagging two native weak learners (threshold stumps on different features)
	using WeakLrnVariant = std::variant<mtrc::TestCl<IrisRec>>;
	std::vector<WeakLrnVariant> models = {};
	WeakLrnVariant model_1 = mtrc::TestCl<IrisRec>(0, false);
	WeakLrnVariant model_2 = mtrc::TestCl<IrisRec>(1, false);
	models.push_back(model_1);
	models.push_back(model_2);

	std::cout << "Bagging native weak learners on Iris: " << std::endl;
	auto startTime = std::chrono::steady_clock::now();
	auto baggingModel_1 = mtrc::Bagging<IrisRec, WeakLrnVariant, mtrc::SubsampleRUS<IrisRec>>(
		10, 0.75, 0.5, {0.5, 0.5}, models); // 50% of each weak learner type
	std::cout << "training... " << std::endl;
	baggingModel_1.train(iris_str, features_iris, response_iris, true);
	auto endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000
			  << " s)" << std::endl;

	baggingModel_1.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Bagging predict on single Iris: " << std::endl;
	vector_print(prediction);

	baggingModel_1.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Bagging predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	// Same ensemble against a deque container
	std::cout << "Bagging native weak learners on deque Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto baggingModel_2 = mtrc::Bagging<IrisRec, WeakLrnVariant, mtrc::SubsampleRUS<IrisRec>>(
		10, 0.75, 0.5, {0.5, 0.5}, models);
	std::cout << "training... " << std::endl;
	baggingModel_2.train(iris_strD, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = "
			  << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000
			  << " s)" << std::endl;

	baggingModel_2.predict(IrisTestRecD, features_iris, prediction);
	std::cout << "Bagging predict on single deque Iris: " << std::endl;
	vector_print(prediction);

	baggingModel_2.predict(IrisTestMultipleRecD, features_iris, prediction);
	std::cout << "Bagging predict on multiple deque Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	return 0;
}
