
#include "../details/classification/metric_classification.hpp"

#include "assets/helpers.cpp" // csv reader

#include <variant>

#include <deque> // for Record test

#include "../details/classification/details/correlation_weighted_accuracy.hpp"



//template<class T>
//using Record = std::vector<T>;


//template<class T>
//using IrisRec = std::deque<T>;


int main()
{

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



	metric::classification::edmClassifier<Record, CC45> wl7 = metric::classification::edmClassifier<Record, CC45>();
	wl7.train(payments, features, response);
	std::vector<bool> r7;
	wl7.predict(test_sample, features, r7);
	std::cout << "\nC4.5 predict: " << r7[0] << std::endl;

	auto cntnr10 = metric::classification::Boosting<Record, metric::classification::edmClassifier<Record, CC45>, metric::classification::SubsampleRUS<Record> >(10, 0.75, 0.5, wl7);
	cntnr10.train(payments, features, response);
	std::vector<bool> r10;
	cntnr10.predict(test_sample, features, r10);
	std::cout << "\nstrong on C4.5 predict: " << r10[0] << std::endl;


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



	//*

	auto wl20 = metric::classification::edmClassifier<IrisRec, libedm::CC45>();
	// usage of weak without strong. Training does not affect further usage of wl20 in strong classifiers!
	wl20.train(iris_str, features_iris, response_iris);
	for (size_t i = 0; i < iris_str.size(); i++) {
		std::vector<bool> r20;
		std::vector<IrisRec> IrisTetRecLocal = { iris_str[i] };
		wl20.predict(IrisTetRecLocal, features_iris, r20);
		std::cout << "\nC4.5 predict on Iris: " << r20[0] << std::endl;
	}

	// using CC45 with default metaparams
	auto cntnr20 = metric::classification::Boosting<IrisRec, metric::classification::edmClassifier<IrisRec, CC45>, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, wl20);
	cntnr20.train(iris_str, features_iris, response_iris, true);
	std::vector<bool> r21;
	cntnr20.predict(IrisTestRec, features_iris, r21);
	std::cout << "\nstrong on C4.5 iris predict: " << r21[0] << std::endl;
	cntnr20.predict(IrisTestMultipleRec, features_iris, r21);
	std::cout << "\nstrong on C4.5 iris predict multiple: " << r21[0] << ", " << r21[1] << ", " << r21[2] << std::endl; // IrisTestMultipleRec
	   

	// using CC45 with metaparams
	auto weak30 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
	auto strong30 = metric::classification::Boosting<IrisRec, metric::classification::edmC45<IrisRec>, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, weak30);
	strong30.train(iris_str, features_iris, response_iris, true);
	std::vector<bool> r30;
	strong30.predict(IrisTestRec, features_iris, r30);
	std::cout << "\nstrong on specialized C4.5 iris predict: " << r30[0] << std::endl;


	//------------------------
	// Bagging_tuple throws an error

	//// using Bagging with tuples
	//using WeakLrnTuple1 = std::tuple<metric::classification::edmC45<IrisRec>, metric::classification::edmClassifier<IrisRec, CC45> >;
	//WeakLrnTuple1 clSet = WeakLrnTuple1(weak30, wl20);
	//auto strong40 = metric::classification::Bagging_tuple<IrisRec, WeakLrnTuple1, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, {0.5, 0.5}, clSet);
	//strong40.train(iris_str, features_iris, response_iris, true);
	//std::vector<bool> r40;
	//strong40.predict(IrisTestRec, features_iris, r40);
	//std::cout << "\nbagging_tuple on both specialized and default c45 iris predict: " << r40[0] << std::endl;

	//------------------------
	// Bagging_tuple throws an error

	//// test tree of learners (boosting strong learner in the role of weak)
	//using WeakLrnTupleTree = std::tuple< metric::classification::Boosting< IrisRec, metric::classification::edmC45<IrisRec>, metric::classification::SubsampleRUS<IrisRec> >, metric::classification::edmC45<IrisRec>, metric::classification::edmClassifier<IrisRec, CC45> >;
	//WeakLrnTupleTree clTree = WeakLrnTupleTree(strong30, weak30, wl20);
	//auto strong50 = metric::classification::Bagging_tuple<IrisRec, WeakLrnTupleTree, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, {0.5, 0.5}, clTree);
	//strong50.train(iris_str, features_iris, response_iris, true);
	//std::vector<bool> r50;
	//strong50.predict(IrisTestRec, features_iris, r50);
	//std::cout << "\nbagging on both boosting and c45 iris predict: " << r50[0] << std::endl;
	//strong50.predict(IrisTestMultipleRec, features_iris, r50);
	//std::cout << "\nbagging_tuple on both boosting and c45 iris predict multiple: " << r50[0] << ", " << r50[1] << ", " << r50[2] << std::endl; // IrisTestMultipleRec

	//* //   correct code, may be enables
	using WeakLrnVariant = std::variant<metric::classification::edmC45<IrisRec>, metric::classification::edmClassifier<IrisRec, CC45> >;
	std::vector<WeakLrnVariant> clSet2 = {};
	WeakLrnVariant weak30_60 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
	WeakLrnVariant wl20_60 = metric::classification::edmClassifier<IrisRec, CC45>();
	clSet2.push_back(weak30_60);
	clSet2.push_back(wl20_60);
	auto strong60 = metric::classification::Bagging<IrisRec, WeakLrnVariant, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, clSet2); // 30% of first weak learner type, 70% of second
	strong60.train(iris_str, features_iris, response_iris, true);
	std::vector<bool> r60;
	strong60.predict(IrisTestRec, features_iris, r60);
	std::cout << "\nbagging_variant on both specialized and default C4.5 iris predict: " << r60[0] << std::endl;
	//*/


	//* //   correct code, may be enables
	using WeakLrnVariantD = std::variant<metric::classification::edmC45<IrisRec>, metric::classification::edmClassifier<IrisRec, CC45> >;
	std::vector<WeakLrnVariantD> clSet3 = {};
	WeakLrnVariantD weak30_65 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
	WeakLrnVariantD wl20_65 = metric::classification::edmClassifier<IrisRec, CC45>();
	clSet3.push_back(weak30_65);
	clSet3.push_back(wl20_65);
	auto strong65 = metric::classification::Bagging<IrisRec, WeakLrnVariantD, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, clSet3); // 30% of first weak learner type, 70% of second
	strong65.train(iris_strD, features_iris, response_iris, true);
	std::vector<bool> r65;
	strong65.predict(IrisTestRecD, features_iris, r65);
	std::cout << "\nbagging_variant with deque on both specialized and default C4.5 iris predict: " << r65[0] << std::endl;
	//*/


	return 0;

}
