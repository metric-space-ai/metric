/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "modules/mapping.hpp"

#include "../../assets/helpers.cpp" // csv reader

#include <variant>

#include <deque> // for Record test






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

	return 0;

}
