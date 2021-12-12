/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#include "modules/mapping/ensembles.hpp"

#include "assets/helpers.cpp" // csv reader

#include <variant>

#include <deque> // for Record test


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
    std::cout << "Boosting example have started" << std::endl;
    std::cout << '\n';

	//*
	typedef std::vector<std::variant<int, double, std::string, std::vector<std::string>, std::vector<double> > > Record1;


	using Record = std::vector<double>;  // may be of arbitrary type, with appropriate accessors



	std::vector<Record> payments = {
		{0,3,5,0},
		{1,4,5,0},
		{2,5,2,1},
		{3,6,2,1}
	};

	std::vector<std::function<double(Record)> > features;


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


	std::vector<Record> test_sample = { {3,6,2, 1} };



	// test on Iris


	using IrisRec = std::vector<std::string>;

	std::vector<IrisRec> iris_str = read_csv<std::vector<IrisRec>>("./assets/iris.csv");
	std::deque<IrisRec> iris_strD = read_csv<std::deque<IrisRec>>("./assets/iris.csv");

	iris_str.erase(iris_str.begin()); // remove headers
	iris_strD.erase(iris_strD.begin()); // remove headers



	std::vector<IrisRec> IrisTestRec = { iris_str[5] }; // 1
	std::deque<IrisRec> IrisTestRecD = { iris_strD[5] }; // 1

	std::vector<IrisRec> IrisTestMultipleRec = { iris_str[5], iris_str[8], iris_str[112] }; // 1, 1, 0

	std::cout << iris_str.size() << std::endl;
	std::cout << iris_str[0].size() << std::endl;


	std::vector<std::function<double(IrisRec)> > features_iris;
	for (int i = 1; i < (int)iris_str[0].size() - 1; ++i) { // skip 1st column
		if (i < (int)iris_str[0].size() - 1) {
			features_iris.push_back(
				[=](auto r) { return std::stod(r[i]); }  // we need closure: [=] instead of [&]
			);
		}
		else { // TODO remove in order to test response accessor
			features_iris.push_back(
				[=](auto r) {
				if (r[i] == "\"setosa\"") {
					//                        cout << r[i] << ", " << i << endl;
					return 1.0;
				}
				else
					return 0.0;
			}
			);
		}
	}

	std::function<bool(IrisRec)> response_iris = [](IrisRec r) {
		if (r[r.size() - 1] == "\"setosa\"") // (std::stod(r[r.size() - 1]) >= 0.5)
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
	// using SVM with default metaparams
	std::cout << "Boost SVM on Iris: " << std::endl;
	auto startTime = std::chrono::steady_clock::now();
	auto svmModel_3 = metric::edmClassifier<IrisRec, CSVM>();
	auto boostSvmModel_3 = metric::Boosting<IrisRec, metric::edmClassifier<IrisRec, CSVM>, metric::SubsampleRUS<IrisRec>>(10, 0.75, 0.5, svmModel_3);
	std::cout << "training... " << std::endl;
	boostSvmModel_3.train(iris_str, features_iris, response_iris, true);
	auto endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	boostSvmModel_3.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Boost SVM predict on single Iris: " << std::endl;
	vector_print(prediction);

	boostSvmModel_3.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Boost SVM predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	////
	// using SVM with metaparams
	std::cout << "Boost specialized SVM on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto svmModel_4 = metric::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
	auto boostSvmModel_4 = metric::Boosting<IrisRec, metric::edmSVM<IrisRec>, metric::SubsampleRUS<IrisRec>>(10, 0.75, 0.5, svmModel_4);
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


	// C4.5

	

	//
	// using C4.5 with default metaparams
	std::cout << "Boosting C4.5 on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto c45Model_2 = metric::edmClassifier<IrisRec, libedm::CC45>();
	auto boostC45Model_2 = metric::Boosting<IrisRec, metric::edmClassifier<IrisRec, CC45>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model_2);
	std::cout << "training... " << std::endl;
	boostC45Model_2.train(iris_str, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	boostC45Model_2.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Boosting C4.5 predict on single Iris: " << std::endl;
	vector_print(prediction);

	boostC45Model_2.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Boosting C4.5 predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";
	   

	//
	// using C4.5 with metaparams
	std::cout << "Boosting with metaparams C4.5 on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto c45Model_3 = metric::edmC45<IrisRec>(2, 1e-3, 0.25, true);
	auto boostC45Model_3 = metric::Boosting<IrisRec, metric::edmC45<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model_3);
	std::cout << "training... " << std::endl;
	boostC45Model_3.train(iris_str, features_iris, response_iris, true);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	boostC45Model_3.predict(IrisTestRec, features_iris, prediction);
	std::cout << "Boosting with metaparams C4.5 predict on single Iris: " << std::endl;
	vector_print(prediction);

	boostC45Model_3.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "Boosting with metaparams C4.5 predict on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	return 0;

}
