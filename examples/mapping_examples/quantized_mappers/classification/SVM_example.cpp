/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/
#include <variant>
#include <chrono>
#include <deque> // for Record test

#include "modules/mapping.hpp"
#include "../../assets/helpers.cpp" // csv reader



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


template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

	std::cout << "[";
	for (int i = 0; i < mat.size(); i++)
	{
		for (int j = 0; j < mat[i].size() - 1; j++)
		{
			std::cout << mat[i][j] << ", ";
		}
		std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
		;
	}
}


int main()
{
	std::cout << "we have started" << std::endl;
	std::cout << '\n';

	using Record = std::vector<int>;  // may be of arbitrary type, with appropriate accessors

	std::vector<Record> payments = {
		{0,3,5,0},
		{1,4,5,0},
		{2,5,2,1},
		{3,6,2,1}
	};

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

	std::vector<Record> test_sample = {
		{0,3,5,0},
		{3,6,2,1}
	};


	std::vector<bool> prediction;
	auto startTime = std::chrono::steady_clock::now();
	auto endTime = std::chrono::steady_clock::now();

	// test on int vector 

	std::cout << "SVM on int vector: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	metric::classification::edmClassifier<Record, CSVM> svmModel_1 = metric::classification::edmClassifier<Record, CSVM>();
	std::cout << "training... " << std::endl;
	svmModel_1.train(payments, features, response);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	svmModel_1.predict(test_sample, features, prediction);
	std::cout << "prediction: " << std::endl;	
	vector_print(prediction);

	std::cout << "\n";

	//*/


	// test on Iris


	using IrisRec = std::vector<std::string>;

	std::vector<IrisRec> iris_str = read_csv<std::vector<IrisRec>>("./assets/iris.csv");
	std::deque<IrisRec> iris_strD = read_csv<std::deque<IrisRec>>("./assets/iris.csv");

	iris_str.erase(iris_str.begin()); // remove headers
	iris_strD.erase(iris_strD.begin()); // remove headers


	std::vector<IrisRec> IrisTestRec = { iris_str[5] }; // 1
	std::deque<IrisRec> IrisTestRecD = { iris_strD[5] }; // 1

	std::vector<IrisRec> IrisTestMultipleRec = { iris_str[5], iris_str[8], iris_str[112] }; // 1, 1, 0
	std::deque<IrisRec> IrisTestMultipleRecD = { iris_str[5], iris_str[8], iris_str[112] }; // 1, 1, 0
	   
	std::vector<std::function<double(IrisRec)> > features_iris;
	for (int i = 1; i < (int)iris_str[0].size() - 1; ++i) { // skip 1st and last column (it is index and label)
		features_iris.push_back(
			[=](auto r) { return std::stod(r[i]); }  // we need closure: [=] instead of [&]
		);
	}

	std::function<bool(IrisRec)> response_iris = [](IrisRec r) {
		if (r[r.size() - 1] == "\"setosa\"")
			return true;
		else
			return false;
	};
	
	//
	// using SVM
	std::cout << "SVM on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto svmModel_2 = metric::classification::edmClassifier<IrisRec, CSVM>();
	std::cout << "training... " << std::endl;
	svmModel_2.train(iris_str, features_iris, response_iris);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	svmModel_2.predict(IrisTestRec, features_iris, prediction);
	std::cout << "SVM prediction on single Iris: " << std::endl;
	vector_print(prediction);

	svmModel_2.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "SVM prediction on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	//
	// using SVM with default metaparams
	std::cout << "Boost SVM on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto svmModel_3 = metric::classification::edmClassifier<IrisRec, CSVM>();
	auto boostSvmModel_3 = metric::Boosting<IrisRec, metric::classification::edmClassifier<IrisRec, CSVM>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel_3);
	std::cout << "training... " << std::endl;
	boostSvmModel_3.train(iris_str, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	boostSvmModel_3.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Boost SVM predict on single Iris: " << std::endl;
	vector_print(prediction);

	boostSvmModel_3.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Boost SVM predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	//
	// using SVM with metaparams
	std::cout << "Boost specialized SVM on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto svmModel_4 = metric::classification::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
	auto boostSvmModel_4 = metric::Boosting<IrisRec, metric::classification::edmSVM<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel_4);
	std::cout << "training... " << std::endl;
	boostSvmModel_4.train(iris_str, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	boostSvmModel_4.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Boost specialized SVM predict on single Iris: " << std::endl;
	vector_print(prediction);

	boostSvmModel_4.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Boost specialized SVM predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";


	//////////////////////////////////////////////




	//
	// using Bagging on both specialized and default SVM
	std::cout << "Bagging on both specialized and default SVM on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	using WeakLrnVariant = std::variant<metric::classification::edmSVM<IrisRec>, metric::classification::edmClassifier<IrisRec, CSVM> >;
	std::vector<WeakLrnVariant> models_1 = {};
	WeakLrnVariant svmModel_5 = metric::classification::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
	WeakLrnVariant svmModel_6 = metric::classification::edmClassifier<IrisRec, CSVM>();
	models_1.push_back(svmModel_5);
	models_1.push_back(svmModel_6);
	auto baggingSVMmodel_1 = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
	std::cout << "training... " << std::endl;
	baggingSVMmodel_1.train(iris_str, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	baggingSVMmodel_1.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Bagging on both specialized and default SVM predict on single Iris: " << std::endl;
	vector_print(prediction);

	baggingSVMmodel_1.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Bagging on both specialized and default SVM predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";


	//
	// using Bagging on both specialized and default SVM with deque
	std::cout << "Bagging on both specialized and default SVM on deque Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto baggingSVMmodel_2 = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
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
	
	return 0;

}
