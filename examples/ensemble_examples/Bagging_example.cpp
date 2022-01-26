/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "metric/mapping/ensembles.hpp"

#include "assets/helpers.cpp"  // csv reader

#include <variant>

#include <deque>  // for Record test

template <typename T>
void vector_print(const std::vector<T> &vec)
{

	std::cout << "[";
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (i < vec.size() - 1)
		{
			std::cout << vec[i] << ", ";
		}
		else
		{
			std::cout << vec[i] << "]" << std::endl;
		}
	}
}

int main()
{
    std::cout << "Bagging example have started" << std::endl;
    std::cout << '\n';

    typedef std::vector<std::variant<int, double, std::string, std::vector<std::string>, std::vector<double>>> Record1;

    using Record = std::vector<double>;  // may be of arbitrary type, with appropriate accessors

    std::vector<Record> payments = { { 0, 3, 5, 0 }, { 1, 4, 5, 0 }, { 2, 5, 2, 1 }, { 3, 6, 2, 1 } };

    std::vector<std::function<double(Record)>> features;

    for (int i = 0; i < (int)payments[0].size() - 1; ++i) {
        features.push_back(
            [=](auto r) { return r[i]; }  // we need closure: [=] instead of [&]   !! THIS DIFFERS FROM API !!
        );
    }

    std::function<bool(Record)> response = [](Record r) {
        if (r[r.size() - 1] >= 0.5)
            return true;
        else
            return false;
    };

    std::vector<Record> test_sample = { { 3, 6, 2, 1 } };

    // test on Iris

    using IrisRec = std::vector<std::string>;

    std::vector<IrisRec> iris_str = read_csv<std::vector<IrisRec>>("./assets/iris.csv");
    std::deque<IrisRec> iris_strD = read_csv<std::deque<IrisRec>>("./assets/iris.csv");

    iris_str.erase(iris_str.begin());  // remove headers
    iris_strD.erase(iris_strD.begin());  // remove headers

    std::vector<IrisRec> IrisTestRec = { iris_str[5] };  // 1
    std::deque<IrisRec> IrisTestRecD = { iris_strD[5] };  // 1

    std::vector<IrisRec> IrisTestMultipleRec = { iris_str[5], iris_str[8], iris_str[112] }; // 1, 1, 0
	std::deque<IrisRec> IrisTestMultipleRecD = { iris_strD[5], iris_strD[8], iris_strD[112] }; // 1, 1, 0

    std::cout << iris_str.size() << std::endl;
    std::cout << iris_str[0].size() << std::endl;

    std::vector<std::function<double(IrisRec)>> features_iris;
    for (int i = 1; i < (int)iris_str[0].size() - 1; ++i) {  // skip 1st column
        if (i < (int)iris_str[0].size() - 1) {
            features_iris.push_back([=](auto r) { return std::stod(r[i]); }  // we need closure: [=] instead of [&]
            );
        } else {  // TODO remove in order to test response accessor
            features_iris.push_back([=](auto r) {
                if (r[i] == "\"setosa\"") {
                    return 1.0;
                } else
                    return 0.0;
            });
        }
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
	
	// SVM


	////
	// using Bagging on both specialized and default SVM
	std::cout << "Bagging on both specialized and default SVM on Iris: " << std::endl;
	auto startTime = std::chrono::steady_clock::now();
	using SVMWeakLrnVariant = std::variant<metric::edmSVM<IrisRec>, metric::edmClassifier<IrisRec, CSVM> >;
	std::vector<SVMWeakLrnVariant> svm_models_1 = {};
	SVMWeakLrnVariant svmModel_5 = metric::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
	SVMWeakLrnVariant svmModel_6 = metric::edmClassifier<IrisRec, CSVM>();
	svm_models_1.push_back(svmModel_5);
	svm_models_1.push_back(svmModel_6);
	auto baggingSVMmodel_1 = metric::Bagging<IrisRec, SVMWeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, svm_models_1); // 30% of first weak learner type, 70% of second
	std::cout << "training... " << std::endl;
	baggingSVMmodel_1.train(iris_str, features_iris, response_iris, true);
	auto endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	baggingSVMmodel_1.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Bagging on both specialized and default SVM predict on single Iris: " << std::endl;
	vector_print(prediction);

	baggingSVMmodel_1.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Bagging on both specialized and default SVM predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";


	////
	// using Bagging on both specialized and default SVM with deque
	std::cout << "Bagging on both specialized and default SVM on deque Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto baggingSVMmodel_2 = metric::Bagging<IrisRec, SVMWeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, svm_models_1); // 30% of first weak learner type, 70% of second
	std::cout << "training... " << std::endl;
	baggingSVMmodel_2.train(iris_strD, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	baggingSVMmodel_2.predict(IrisTestRecD, features_iris, prediction);
	std::cout << "Bagging on both specialized and default SVM predict on single deque Iris: " << std::endl;
	vector_print(prediction);

	baggingSVMmodel_2.predict(IrisTestMultipleRecD, features_iris, prediction);
	std::cout << "Bagging on both specialized and default SVM predict on multiple deque Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";


	// C4.5
	
	//
	// using Bagging on both specialized and default C4.5
	std::cout << "Bagging on both specialized and default C4.5 on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	using C45WeakLrnVariant = std::variant<metric::edmC45<IrisRec>, metric::edmClassifier<IrisRec, CC45> >;
	std::vector<C45WeakLrnVariant> c45_models_1 = {};
	C45WeakLrnVariant c45Model_4 = metric::edmC45<IrisRec>(2, 1e-3, 0.25, true);
	C45WeakLrnVariant c45Model_5 = metric::edmClassifier<IrisRec, CC45>();
	c45_models_1.push_back(c45Model_4);
	c45_models_1.push_back(c45Model_5);
	auto baggingC45model_1 = metric::Bagging<IrisRec, C45WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, c45_models_1); // 30% of first weak learner type, 70% of second
	std::cout << "training... " << std::endl;
	baggingC45model_1.train(iris_str, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;
	
	baggingC45model_1.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Bagging on both specialized and default C4.5 predict on single Iris: " << std::endl;
	vector_print(prediction);

	baggingC45model_1.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Bagging on both specialized and default C4.5 predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";


	//
	// using Bagging on both specialized and default C4.5 with deque
	std::cout << "Bagging on both specialized and default C4.5 on deque Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto baggingC45model_2 = metric::Bagging<IrisRec, C45WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, c45_models_1); // 30% of first weak learner type, 70% of second
	std::cout << "training... " << std::endl;
	baggingC45model_2.train(iris_strD, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	baggingC45model_2.predict(IrisTestRecD, features_iris, prediction);
	std::cout << "Bagging on both specialized and default C4.5 predict on single deque Iris: " << std::endl;
	vector_print(prediction);

	baggingC45model_2.predict(IrisTestMultipleRecD, features_iris, prediction);
	std::cout << "Bagging on both specialized and default C4.5 predict on multiple deque Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

    return 0;
}
