/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/
#include <variant>
#include <chrono>
#include <deque> // for Record test

#include "../../assets/helpers.cpp" // csv reader
#include "metric/mapping.hpp"

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
	std::cout << "SVN example have started" << std::endl;
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
	metric::edmClassifier<Record, CSVM> svmModel_1 = metric::edmClassifier<Record, CSVM>();
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
	

	std::cout << "SVM on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto svmModel_2 = metric::edmClassifier<IrisRec, CSVM>();
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

	
	return 0;

}
