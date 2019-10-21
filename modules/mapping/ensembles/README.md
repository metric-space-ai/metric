
## Ensembling

For all below examples of the bagging and boosting suppose we have Iris dataset:

Here is our loaded dataset:

```cpp
using IrisRec = std::vector<std::string>;

std::vector<IrisRec> iris_str = read_csv<std::vector<IrisRec>>("./assets/iris.csv");

iris_str.erase(iris_str.begin()); // remove headers
```

Here will be our test samples, one is a single sample and another is multiple samples:

```cpp
std::vector<IrisRec> IrisTestRec = { iris_str[5] }; // 1
std::vector<IrisRec> IrisTestMultipleRec = { iris_str[5], iris_str[8], iris_str[112] }; // 1, 1, 0
```

Then we should define features and responce (first except last elements is features, last one is a label):

```cpp
std::vector<std::function<double(IrisRec)> > features_iris;
for (int i = 1; i < (int)iris_str[0].size() - 1; ++i) { // skip 1st and last column (it is index  and label)
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
```



#### Boosting

###### SVM

On the same Iris dataset we can define and train Boosting model.

SVM with default metaparams:
```cpp
auto svmModel = metric::edmClassifier<IrisRec, CSVM>();
auto boostSvmModel = metric::Boosting<IrisRec, metric::edmClassifier<IrisRec, CSVM>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel);
boostSvmModel.train(iris_str, features_iris, response_iris, true);

boostSvmModel.predict(IrisTestRec, features_iris, prediction);
// out
// Boost SVM predict on single Iris:
// [1]

boostSvmModel.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost SVM predict on multiple Iris:
// [1, 1, 0]
```
 
 

SVM with specialized metaparams:
```cpp
auto svmModel = metric::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
auto boostSvmModel = metric::Boosting<IrisRec, metric::edmSVM<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel);
boostSvmModel.train(iris_str, features_iris, response_iris, true);

boostSvmModel.predict(IrisTestRec, features_iris, prediction);
// out
// Boost specialized SVM predict on single Iris:
// [1]

boostSvmModel.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost specialized SVM predict on multiple Iris:
// [1, 1, 0]
```

---

###### C4.5

C4.5 with default metaparams
```cpp
auto c45Model = metric::edmClassifier<IrisRec, libedm::CC45>();
auto boostC45Model = metric::Boosting<IrisRec, metric::edmClassifier<IrisRec, CC45>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model);
boostC45Model.train(iris_str, features_iris, response_iris, true);

boostC45Model.predict(IrisTestRec, features_iris, prediction);
// out
// Boost C4.5 predict on single Iris:
// [1]

boostC45Model.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost C4.5 predict on multiple Iris:
// [1, 1, 0]
```

C4.5 with specialized metaparams
```cpp
auto c45Model = metric::edmC45<IrisRec>(2, 1e-3, 0.25, true);
auto boostC45Model = metric::Boosting<IrisRec, metric::edmC45<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model);
boostC45Model.train(iris_str, features_iris, response_iris, true);

boostC45Model.predict(IrisTestRec, features_iris, prediction);
// out
// Boost specialized C4.5 predict on single Iris:
// [1]

boostC45Model.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost specialized C4.5 predict on multiple Iris:
// [1, 1, 0]
```

*For a full example and more details see `examples/ensemble_examples/Boosting_example.cpp`*

---

#### Bagging

###### SVM

Vector with defined models
```cpp
using WeakLrnVariant = std::variant<metric::edmSVM<IrisRec>, metric::edmClassifier<IrisRec, CSVM> >;
std::vector<WeakLrnVariant> svmModels = {};
WeakLrnVariant svmModel_1 = metric::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
WeakLrnVariant svmModel_2 = metric::edmClassifier<IrisRec, CSVM>();
svmModels.push_back(svmModel_1);
svmModels.push_back(svmModel_2);
```

Bagging on both specialized and default SVM
```cpp
auto baggingSVMmodel = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, svmModels); // 30% of first weak learner type, 70% of second
baggingSVMmodel.train(iris_str, features_iris, response_iris, true);

baggingSVMmodel.predict(IrisTestRec, features_iris, prediction);
// out
// Bagging SVM predict on single Iris:
// [1]

baggingSVMmodel.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Bagging SVM predict on multiple Iris:
// [1, 1, 0]
```

---

###### C4.5

Vector with defined models
```cpp
using WeakLrnVariant = std::variant<metric::edmC45<IrisRec>, metric::edmClassifier<IrisRec, CC45> >;
std::vector<WeakLrnVariant> c45Models = {};
WeakLrnVariant c45Model_1 = metric::edmC45<IrisRec>(2, 1e-3, 0.25, true);
WeakLrnVariant c45Model_2 = metric::edmClassifier<IrisRec, CC45>();
c45Models.push_back(c45Model_1);
c45Models.push_back(c45Model_2);
```

Bagging on both specialized and default C4.5
```cpp
auto baggingC45model = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, c45Models); // 30% of first weak learner type, 70% of second
baggingC45model.train(iris_str, features_iris, response_iris, true);

baggingC45model.predict(IrisTestRec, features_iris, prediction);
// out
// Bagging C4.5 predict on single Iris:
// [1]

baggingC45model.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Bagging C4.5 predict on multiple Iris:
// [1, 1, 0]
```

*For a full example and more details see `examples/ensemble_examples/Bagging_example.cpp`*

---

#### Correlation Weighted Accuracy


Suppose we have the following sets:
```cpp
std::vector<int> g1 = {3, 2, 2, 3, 1, 1}; // Known groups
std::vector<int> g2 = {4, 2, 2, 2, 1, 1}; // Predicted groups
```

We can calculate Correlation Weighted Accuracy for that:
```cpp
double cwa = metric::correlation_weighted_accuracy(g1, g2);
// out
// Correlation Weighted Accuracy:
// 0.637323
```

*For a full example and more details see `examples/ensemble_examples/CWA_example.cpp`*

---

#### Decision Tree


Suppose we have the following data:
```cpp

    typedef std::variant<double, std::vector<double>, std::vector<std::vector<double>>, std::string> V;  // field type
    typedef std::vector<V> Record;

    std::vector<std::vector<double>> img1 = { // needs to be larger than blur kernel size coded intarnally as 11
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };
    std::vector<std::vector<double>> img2
        = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
              { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

    std::vector<Record> selection
        = { { V((double)2), V(std::vector<double>({ 1, 2 })), V(std::vector<double>({ 0, 1, 1, 1, 1, 1, 2, 3 })),
                V(img1), V(""), V((double)1) },
              { V((double)2), V(std::vector<double>({ 1, 5 })), V(std::vector<double>({ 1, 1, 1, 1, 1, 2, 3, 4 })),
                  V(img2), V("A"), V((double)1) },
              { V((double)1), V(std::vector<double>({ 4, 5 })), V(std::vector<double>({ 2, 2, 2, 1, 1, 2, 0, 0 })),
                  V(img2), V("AA"), V((double)2) },
              { V((double)2), V(std::vector<double>({ 1, 2 })), V(std::vector<double>({ 3, 3, 2, 2, 1, 1, 0, 0 })),
                  V(img1), V("AAA"), V((double)1) },
              { V((double)2), V(std::vector<double>({ 5 })), V(std::vector<double>({ 4, 3, 2, 1, 0, 0, 0, 0 })),
                  V(img1), V("AAAA"), V((double)1) },
              { V((double)2), V(std::vector<double>({ 1, 4, 5 })), V(std::vector<double>({ 4, 3, 2, 1, 0, 0, 0, 0 })),
                  V(img2), V("BAAA"), V((double)1) },
              { V((double)1), V(std::vector<double>({ 1, 2, 3, 4 })),
                  V(std::vector<double>({ 5, 3, 2, 1, 0, 0, 0, 0 })), V(img2), V("BBAA"), V((double)3) },
              { V((double)1), V(std::vector<double>({ 1 })), V(std::vector<double>({ 4, 6, 2, 2, 1, 1, 0, 0 })),
                  V(img1), V("BBA"), V((double)1) },
              { V((double)2), V(std::vector<double>({ 4, 5 })), V(std::vector<double>({ 3, 7, 2, 1, 0, 0, 0, 0 })),
                  V(img2), V("BB"), V((double)1) },
              { V((double)2), V(std::vector<double>({ 1, 2, 4, 5 })),
                  V(std::vector<double>({ 2, 5, 1, 1, 0, 0, 1, 2 })), V(img1), V("B"), V((double)1) } };
```

Then we should create accessors:
```cpp

    // vector of accessors for field 0
    auto field0accessors = [](const Record& r) { return std::get<double>(r[0]); };

    // vector of accessors for field 1
    auto field1accessors = [](const Record& r) {
        std::vector<double> v(std::get<std::vector<double>>(r[1]));
        v.resize(4);
        return v;
    };

    // vector of accessors for field 2
    auto field2accessors = [](const Record& r) {
        std::vector<double> v(std::get<std::vector<double>>(r[2]));
        v.resize(8);
        return v;
    };

    // vector of accessors for field 3
    auto field3accessors = [](const Record& r) { return std::get<std::vector<std::vector<double>>>(r[3]); };

    // vector of accessors for field 4
    auto field4accessors = [](const Record& r) { return std::get<std::string>(r[4]); };

    // label accessor (for single record)
    std::function<int(const Record&)> response = [](const Record& r) { return (int)std::abs(std::get<double>(r[5])); };

    // build dimension and Dimension objects

    typedef double InternalType;

    // features
    using a0_type = decltype(field0accessors);
    using a1_type = decltype(field1accessors);
    using a2_type = decltype(field2accessors);
    using a3_type = decltype(field3accessors);
    using a4_type = decltype(field4accessors);

    auto dim0 = metric::make_dimension(metric::Euclidian<InternalType>(), field0accessors);
    auto dim1 = metric::make_dimension(metric::Manhatten<InternalType>(), field1accessors);
    auto dim2 = metric::make_dimension(metric::P_norm<InternalType>(), field2accessors);
    auto dim3 = metric::make_dimension(metric::Euclidian_thresholded<InternalType>(), field2accessors);
    auto dim4 = metric::make_dimension(metric::Cosine<InternalType>(), field2accessors);
    auto dim5 = metric::make_dimension(metric::SSIM<double, std::vector<InternalType>>(), field3accessors);
    auto dim6 = metric::make_dimension(metric::TWED<InternalType>(0, 1), field2accessors);
    auto dim7 = metric::make_dimension(metric::Edit<char>(), field4accessors);
    auto dim10 = metric::make_dimension(metric::EMD<InternalType>(8, 8), field2accessors);

    typedef std::variant<metric::Dimension<metric::Euclidian<InternalType>, a0_type>,
        metric::Dimension<metric::Manhatten<InternalType>, a1_type>,
        metric::Dimension<metric::P_norm<InternalType>, a2_type>,
        metric::Dimension<metric::Euclidian_thresholded<InternalType>, a2_type>,
        metric::Dimension<metric::Cosine<InternalType>, a2_type>,
        metric::Dimension<metric::SSIM<double, std::vector<InternalType>>, a3_type>,
        metric::Dimension<metric::TWED<InternalType>, a2_type>,
        metric::Dimension<metric::EMD<InternalType>, a2_type>,  // matrix C is temporary created inside functor
        metric::Dimension<metric::Edit<std::string::value_type>, a4_type>>
        VariantType;

    std::vector<VariantType> dims = { dim0, dim1, dim2, dim3, dim4, dim5, dim6, dim7, dim10 };
```


Then we are ready to create and train Decision Tree model:

```cpp
auto model = metric::DT<Record>();
model.train(selection, dims, response);
```

Using the trained model we can make predictions:

```cpp

std::vector<Record> test_sample = { selection[0], selection[2], selection[6] };
std::vector<int> prediction;
model.predict(test_sample, dims, prediction);

// out
// Metric Desicion Tree prediction:
// [1, 1, 3]
```

*For a full example and more details see `examples/ensemble_examples/MetricDecisionTree_example.cpp`*

---

## Run
*You need STL and C++17 support to compile.*

METRIC | MAPPING works headonly. Just include the header into your project.

```cpp
#include "modules/mapping.hpp"
```

or directly include one of specified distance from the following:

```cpp
#include "modules/mapping/ensembles.hpp"
```

#### Using CMake

_Windows_

```bash
mkdir build
cd build
cmake .. -A x64 -T llvm
```
Then open solution in the Microsoft Visual Studio

_Linux_

Just run cmake
```bash
mkdir build
cd build
cmake ..
make
```

#### Direct compilation, using compiler


```bash
$ clang++ ./examples/ensemble_examples/Boosting_example.cpp -std=c++17
```

# Clone and build

After cloning repository run next command to update submodules:

`$ git submodule init
 $ git submodule update`
 
 
