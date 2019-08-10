/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include "modules/mapping.hpp"
#include "modules/mapping/ensembles/DT/correlation_weighted_accuracy.hpp"
#include "../../assets/helpers.cpp" // csv reader

#include <variant>

#include <deque> // for Record test


    int test_CWA()
    {

        std::vector<int> g1 = {3, 2, 2, 3, 1, 1}; // Known groups
        std::vector<int> g2 = {4, 2, 2, 2, 1, 1}; // Predicted groups

        double cwa = metric::correlation_weighted_accuracy(g1,g2);

        std::cout << cwa << std::endl;

        return 0;
    }



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


    for (int i=0; i<(int)payments[0].size() - 1; ++i) {
        features.push_back(
            [=](auto r) { return r[i]; }  // we need closure: [=] instead of [&]   !! THIS DIFFERS FROM API !!
        );
    }


    std::function<bool(Record)> response = [](Record r) {
        if(r[r.size() - 1] >= 0.5)
            return true;
        else
            return false;
    };


    std::vector<Record> test_sample = {{3,6,2, 1}};



    // Usage of RUSBoost

    metric::TestCl<Record> wl4 = metric::TestCl<Record>(0, false);

    wl4.train(payments, features, response);
    std::vector<bool> r4;
    wl4.predict(test_sample, features, r4);
    std::cout << "\nweak predict: " << r4[0] << std::endl << std::endl;

    auto cntnr4 = metric::Boosting<Record, metric::TestCl<Record>, metric::SubsampleRUS<Record> >(10, 0.75, 0.5, wl4);
    cntnr4.train(payments, features, response);
    std::vector<bool> r5;
    cntnr4.predict(test_sample, features, r5);
    std::cout << "\nstrong predict: " << r5[0] << std::endl;


    metric::classification::edmClassifier<Record, CC45> wl7 = metric::classification::edmClassifier<Record, CC45>();
    wl7.train(payments, features, response);
    std::vector<bool> r7;
    wl7.predict(test_sample, features, r7);
    std::cout << "\nC4.5 predict: " << r7[0] << std::endl;

    metric::classification::edmClassifier<Record, CSVM> wl8 = metric::classification::edmClassifier<Record, CSVM>();
    wl8.train(payments, features, response);
    std::vector<bool> r8;
    wl8.predict(test_sample, features, r8);
    std::cout << "\nSVM predict: " << r8[0] << std::endl;

    auto cntnr10 = metric::Boosting<Record, metric::classification::edmClassifier<Record, CC45>, metric::SubsampleRUS<Record> >(10, 0.75, 0.5, wl7);
    cntnr10.train(payments, features, response);
    std::vector<bool> r10;
    cntnr10.predict(test_sample, features, r10);
    std::cout << "\nstrong on c45 predict: " << r10[0] << std::endl;


    //*/


    // test on Iris


    using IrisRec = std::vector<std::string>;

    std::vector<IrisRec> iris_str = read_csv<std::vector<IrisRec>>("./assets/iris.csv");
    std::deque<IrisRec> iris_strD = read_csv<std::deque<IrisRec>>("./assets/iris.csv");

    iris_str.erase(iris_str.begin()); // remove headers
    iris_strD.erase(iris_strD.begin()); // remove headers



    std::vector<IrisRec> IrisTestRec = {iris_str[5]}; // 1
    std::deque<IrisRec> IrisTestRecD = {iris_strD[5]}; // 1

    std::vector<IrisRec> IrisTestMultipleRec = {iris_str[5], iris_str[8], iris_str[112]}; // 1, 1, 0

    std::cout << iris_str.size() << std::endl;
    std::cout << iris_str[0].size() << std::endl;


    std::vector<std::function<double(IrisRec)> > features_iris;
    for (int i=1; i<(int)iris_str[0].size() - 1; ++i) { // skip 1st column
        if (i < (int)iris_str[0].size()-1) {
            features_iris.push_back(
                [=](auto r) { return std::stod(r[i]); }  // we need closure: [=] instead of [&]
            );
        } else { // TODO remove in order to test response accessor
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


    for (size_t i=0; i<iris_str[0].size()-2; i++)
        std::cout << features_iris[i](iris_str[10]) << ", ";
    std::cout << std::endl;



    //*

    auto wl20 = metric::classification::edmClassifier<IrisRec, libedm::CC45>();
    // usage of weak without strong. Training does not affect further usage of wl20 in strong classifiers!
    wl20.train(iris_str, features_iris, response_iris);
    for (size_t i=0; i<iris_str.size(); i++) {
        std::vector<bool> r20;
        std::vector<IrisRec> IrisTetRecLocal = {iris_str[i]};
        wl20.predict(IrisTetRecLocal, features_iris, r20);
        std::cout << "\nC4.5 predict on Iris: " << r20[0] << std::endl;
    }

    // using CC45 with default metaparams
    auto cntnr20 = metric::Boosting<IrisRec, metric::classification::edmClassifier<IrisRec, CC45>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, wl20);
    cntnr20.train(iris_str, features_iris, response_iris, true);
    std::vector<bool> r21;
    cntnr20.predict(IrisTestRec, features_iris, r21);
    std::cout << "\nstrong on c45 iris predict: " << r21[0] << std::endl;
    cntnr20.predict(IrisTestMultipleRec, features_iris, r21);
    std::cout << "\nstrong on c45 iris predict multiple: " << r21[0] << ", " << r21[1] << ", " << r21[2] << std::endl; // IrisTestMultipleRec

    //*/

    // using CSVM with default metaparams
    auto wl25 = metric::classification::edmClassifier<IrisRec, CSVM>();
    auto cntnr25 = metric::Boosting<IrisRec, metric::classification::edmClassifier<IrisRec, CSVM>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, wl25);
    cntnr25.train(iris_str, features_iris, response_iris, true);
    std::vector<bool> r25;
    cntnr20.predict(IrisTestRec, features_iris, r25);
    std::cout << "\nstrong on CSVM iris predict: " << r25[0] << std::endl;


    // using CC45 with metaparams
    auto weak30 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
    auto strong30 = metric::Boosting<IrisRec, metric::classification::edmC45<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, weak30);
    strong30.train(iris_str, features_iris, response_iris, true);
    std::vector<bool> r30;
    strong30.predict(IrisTestRec, features_iris, r30);
    std::cout << "\nstrong on specialized c45 iris predict: " << r30[0] << std::endl;


    // using CSVM with metaparams
    auto weak35 = metric::classification::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
    auto strong35 = metric::Boosting<IrisRec, metric::classification::edmSVM<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, weak35);
    strong35.train(iris_str, features_iris, response_iris, true);
    std::vector<bool> r35;
    strong30.predict(IrisTestRec, features_iris, r35);
    std::cout << "\nstrong on specialized SVM iris predict: " << r35[0] << std::endl;

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
    auto strong60 = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, {0.3, 0.7}, clSet2); // 30% of first weak learner type, 70% of second
    strong60.train(iris_str, features_iris, response_iris, true);
    std::vector<bool> r60;
    strong60.predict(IrisTestRec, features_iris, r60);
    std::cout << "\nbagging_variant on both specialized and default c45 iris predict: " << r60[0] << std::endl;
    //*/


    //* //   correct code, may be enables
    using WeakLrnVariantD = std::variant<metric::classification::edmC45<IrisRec>, metric::classification::edmClassifier<IrisRec, CC45> >;
    std::vector<WeakLrnVariantD> clSet3 = {};
    WeakLrnVariantD weak30_65 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
    WeakLrnVariantD wl20_65 = metric::classification::edmClassifier<IrisRec, CC45>();
    clSet3.push_back(weak30_65);
    clSet3.push_back(wl20_65);
    auto strong65 = metric::Bagging<IrisRec, WeakLrnVariantD, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, {0.3, 0.7}, clSet3); // 30% of first weak learner type, 70% of second
    strong65.train(iris_strD, features_iris, response_iris, true);
    std::vector<bool> r65;
    strong65.predict(IrisTestRecD, features_iris, r65);
    std::cout << "\nbagging_variant with deque on both specialized and default c45 iris predict: " << r65[0] << std::endl;
    //*/



    /* //  correct code, may be enabled
    // using bagging_variant as weak for boosting (testing clone())
    using WeakLrnVariantD = std::variant<metric::classification::edmC45<IrisRec>, metric::classification::edmClassifier<IrisRec, CC45> >;
    using BaggingType = metric::classification::Bagging<IrisRec, WeakLrnVariantD, metric::classification::SubsampleRUS<IrisRec> >;
    std::vector<WeakLrnVariantD> clSet70 = {};
    WeakLrnVariantD weak30_70 = metric::classification::edmC45<IrisRec>(2, 1e-3, 0.25, true);
    WeakLrnVariantD wl20_70 = metric::classification::edmClassifier<IrisRec, CC45>();
    clSet70.push_back(weak30_70);
    clSet70.push_back(wl20_70);
    auto strong69 = BaggingType(10, 0.75, 0.5, {0.3, 0.7}, clSet70); // 30% of first weak learner type, 70% of second
    auto strong70 = metric::classification::Boosting<IrisRec, BaggingType, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, strong69);
    strong70.train(iris_strD, features_iris, response_iris, true);
    std::vector<bool> r70;
    strong70.predict(IrisTestRecD, features_iris, r70);
    std::cout << "\nboosting on bagging_variant iris predict: " << r70[0] << std::endl;
    //*/



    test_CWA();





    // test: EDM without wrappers, reproducing single sample misclassification

    /*

    CDataset dataset = wl20.read_data(iris_str, features_iris);

//    CC45 model = CC45(dataset);
    shared_ptr<CC45> model = make_shared<CC45>(dataset);
    auto pred = model->Classify(dataset);

    auto result = pred->GetPredictedLabelIndices();

    auto m = dataset.GetData();
    auto probs = pred->GetProbs();
    auto corr = pred->GetCorrectness();
    auto acc = pred->GetAccuracy();
    auto s = dataset.GetInfo();

    for (int i=0; i<iris_str.size(); i++) {
        std::vector<IrisRec> sub = { iris_str[i], iris_str[i], iris_str[i], iris_str[i] };

        CDataset subset = wl20.read_data(sub, features_iris);
        auto pred = model->Classify(subset);
        auto result = pred->GetPredictedLabelIndices();

        auto m = subset.GetData();
        auto probs = pred->GetProbs();
        auto corr = pred->GetCorrectness();
        auto acc = pred->GetAccuracy();
        auto s = subset.GetInfo();

        cout << result[3] << endl;

    }

    */



	// test EMD separately

	typedef float InputType;

	typedef std::variant<double, std::vector<int>, int> F; // field type
	typedef std::vector<F> R;
	std::vector<R> dataset = {
		{F(std::vector<int>({1, 0, 0})),
		 F((int)0) // label
		},
		{F(std::vector<int>({0, 2, 0})), // from example
		 F((int)1)
		}
	};


	auto a0 = [](const R & r) {
		auto & v = std::get<std::vector<int>>(r[0]);
		return std::vector<InputType>(v.begin(), v.end());
	};

	// label accessor (for single record)
	std::function<int(R)> resp = [](R r)
	{
		return (int)std::abs(std::get<int>(r[1]));
	};


	std::vector<std::vector<InputType>> C = { {0, 10, 20}, {10, 0, 10}, {20, 10, 0} }; // symmetric



	metric::Dimension<metric::EMD<InputType>, decltype(a0)> dimEMD(a0, metric::EMD<InputType>(3, 3));
	auto emd_dist = dimEMD.get_distance(dataset[0], dataset[1]);

	metric::EMD<InputType> EMD_functor(C);

	auto v1i = std::get<std::vector<int>>(dataset[0][0]);
	auto v2i = std::get<std::vector<int>>(dataset[1][0]);
	std::vector<InputType> v1t(v1i.begin(), v1i.end());
	std::vector<InputType> v2t(v2i.begin(), v2i.end());

	auto emd_dist_C = EMD_functor(v1t, v2t);

	std::cout << "\nEMD distance 1: " << emd_dist << " " << emd_dist_C << "\n";





	//    test EMD separately

	std::vector<InputType> v1 = { 0, 2, 0, 0, 0, 0 };
	std::vector<InputType> v2 = { 2, 0, 1, 0, 0, 0 };
	std::vector<std::vector<InputType>> CC = { {0, 10, 20, 30, 40, 50},
											 {10, 0, 10, 20, 30, 40},
											 {20, 10, 0, 10, 20, 30},
											 {30, 20, 10, 0, 10, 20},
											 {40, 30, 20, 10, 0, 10},
											 {50, 40, 30, 20, 10, 0} }; // symmetric
	auto emd_dist_2 = metric::EMD<InputType>(CC)(v1, v2);

	auto emd_dist_3 = metric::EMD<InputType>(6, 6)(v1, v2);

	std::cout << "\nEMD distance 2 : " << emd_dist_2 << " " << emd_dist_3 << "\n";

    return 0;

}
