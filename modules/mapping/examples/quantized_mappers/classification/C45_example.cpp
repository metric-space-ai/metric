#include <variant>
#include <chrono>
#include <deque> // for Record test

#include "../details/classification/metric_classification.hpp"
#include "assets/helpers.cpp" // csv reader
#include "../details/classification/details/correlation_weighted_accuracy.hpp"


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
	
	std::cout << "C4.5 on int vector: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	metric::classification::edmClassifier<Record, CC45> c45Model_1 = metric::classification::edmClassifier<Record, CC45>();
	std::cout << "training... " << std::endl;
	c45Model_1.train(payments, features, response);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;
	
	c45Model_1.predict(test_sample, features, prediction);
	std::cout << "C4.5 prediction: " << std::endl;
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
		if (i < (int)iris_str[0].size() - 1) {
			features_iris.push_back(
				[=](auto r) { return std::stod(r[i]); }  // we need closure: [=] instead of [&]
			);
		}
		else { // TODO remove in order to test response accessor
			features_iris.push_back(
				[=](auto r) {
				if (r[i] == "\"setosa\"") {
					return 1.0;
				}
				else
					return 0.0;
			}
			);
		}
	}

	std::function<bool(IrisRec)> response_iris = [](IrisRec r) {
		if (r[r.size() - 1] == "\"setosa\"") 
			return true;
		else
			return false;
	};




	//
	// using C4.5
	std::cout << "C4.5 on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto c45Model_2 = metric::classification::edmClassifier<IrisRec, libedm::CC45>();
	// usage of weak without strong. Training does not affect further usage of wl20 in strong classifiers!
	std::cout << "training... " << std::endl;
	c45Model_2.train(iris_str, features_iris, response_iris);
	endTime = std::chrono::steady_clock::now();
	std::cout << "trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	c45Model_2.predict(IrisTestRec, features_iris, prediction);
	std::cout << "C4.5 prediction on single Iris: " << std::endl;
	vector_print(prediction);

	c45Model_2.predict(IrisTestMultipleRec, features_iris, prediction);
	std::cout << "C4.5 prediction on multiple Iris: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";


	//
	// using C4.5 with default metaparams
	std::cout << "Boosting C4.5 on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto boostC45Model_2 = metric::classification::Boosting<IrisRec, metric::classification::edmClassifier<IrisRec, CC45>, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model_2);
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
	auto c45Model_3 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
	auto boostC45Model_3 = metric::classification::Boosting<IrisRec, metric::classification::edmC45<IrisRec>, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model_3);
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

	
	//
	// using Bagging on both specialized and default C4.5
	std::cout << "Bagging on both specialized and default C4.5 on Iris: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	using WeakLrnVariant = std::variant<metric::classification::edmC45<IrisRec>, metric::classification::edmClassifier<IrisRec, CC45> >;
	std::vector<WeakLrnVariant> models_1 = {};
	WeakLrnVariant c45Model_4 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
	WeakLrnVariant c45Model_5 = metric::classification::edmClassifier<IrisRec, CC45>();
	models_1.push_back(c45Model_4);
	models_1.push_back(c45Model_5);
	auto baggingC45model_1 = metric::classification::Bagging<IrisRec, WeakLrnVariant, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
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
	auto baggingC45model_2 = metric::classification::Bagging<IrisRec, WeakLrnVariant, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
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
