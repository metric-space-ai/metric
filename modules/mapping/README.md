# METRIC | MAPPING

## Overview


## Examples

##### Quantized mapper (encoder only) 



1. No information
    - kmedoids (with kmeans as specialization) | based on d-full matrices
    - DBSCAN | based on d-full matrices
    - Affinity propagation | based on d-full matrices
    - Hierarchical Clustering | based on d-minimal tree


2. Few constrains
    - Laplacian optimization (spectral clustering) | based on d-full matrices
    - Aggregate | based on d-minimal tree


3. Training data
    - SVM
    - C4.5
    - Decision Tree


##### Continuous mapper (encoder/decoder)



1. No information
    - SOM (k-related space to k-structured space converter)
    - PCAnet (k-structured space to k-related space converter)


2. Training data
    - ESN (k-structured space to k-related space)

---

#### K-Medoids

Suppose we have the following records:
```cpp
std::vector<std::vector<float>> data{
		   {0, 0, 0, 0, 0},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000} };
```

And here is example of how can be found clusters using K-Medoids:
```cpp
auto[assignments, seeds, counts] = metric::kmedoids(data, 4);
// out:

//assignments:
//2, 0, 3, 0, 1, 0

//seeds:
//5, 4, 0, 2

//counts:
//3, 1, 1, 1
```

---

#### K-Means

Suppose we have the following records:
```cpp
std::vector<std::vector<float>> data{
		   {0, 0, 0, 0, 0},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000} };
```

And here is example of how can be found clusters using K-Means:
```cpp
auto[assignments, means, counts] = metric::kmeans(data, 4); 
// out:

//assignments:
//0, 1, 2, 3, 2, 3

//means:
//9.69069, 16.2635, 4.74928, 232.576, 232.576
//0, 0, 0, 0, 0
//4.00673, 4.26089, 3.72081, 96.1618, 96.1618
//3.4824, 8.15624, -0.185407, 83.5776, 83.5776

//counts:
//2, 1, 2, 1
```

---

#### DBSCAN

Suppose we have the following records:
```cpp
std::vector<std::vector<float>> data{
		   {0, 0, 0, 0, 0},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000} };
```

And here is example of how can be found clusters using DBSCAN:
```cpp
auto[assignments, seeds, counts] = metric::dbscan(data, (float) 64.0, 1); 
// out:

//assignments:
//1, 1, 2, 1, 3, 1

//seeds:
//0, 2, 4

//counts:
//4, 1, 1
```

---

#### Affinity Propagation

Suppose we have the following records:
```cpp
std::vector<std::vector<float>> data{
		   {0, 0, 0, 0, 0},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000} };
```

And here is example of how can be found clusters using Affinity Propagation:
```cpp
auto[assignments, exemplars, counts] = metric::affprop(data);
// out:

//assignments:
//1, 1, 0, 1, 0, 1

//exemplars:
//4, 5

//counts:
//2, 4
```

---

#### Hierarchical Clustering

t.b.d.

---

#### Laplacian optimization
---

#### Aggregate
---


#### Metric Decision Tree
---

#### SVM

#### *First example*

Suppose we have set of payments data, where each payment is set of ints:
```cpp
using Record = std::vector<int>;

std::vector<Record> payments = {
	{0,3,5,0},
	{1,4,5,0},
	{2,5,2,1},
	{3,6,2,1}
};
```

Then we defined features and responce (first three elements is features, last one is a label):

```cpp
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
```

And we can define and train SVM model:

```cpp
metric::edmClassifier<Record, CSVM> svmModel_1 = metric::edmClassifier<Record, CSVM>();
svmModel_1.train(payments, features, response);
```

Once model will been trained we can make predict on a test sample:

```cpp
std::vector<Record> test_sample = {
	{0,3,5,0},
	{3,6,2,1}
};

svmModel_1.predict(test_sample, features, prediction);
// out
// prediction:
// [0, 1]
```

#### *Iris example*

We can run SVM on famous Iris dataset.

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


###### Simple model

So, we are ready to define and train simple SVM model: 
```cpp
auto svmModel_2 = metric::edmClassifier<IrisRec, CSVM>();
svmModel_2.train(iris_str, features_iris, response_iris);

svmModel_2.predict(IrisTestRec, features_iris, prediction);
// out
// SVM prediction on single Iris:
// [1]

svmModel_2.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// SVM prediction on multiple Iris:
// [1, 1, 0]
```
 
 
###### Boosting

On the same Iris dataset we can define and train Boosting model.

SVM with default metaparams:
```cpp
auto svmModel_3 = metric::edmClassifier<IrisRec, CSVM>();
auto boostSvmModel_3 = metric::Boosting<IrisRec, metric::edmClassifier<IrisRec, CSVM>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel_3);
boostSvmModel_3.train(iris_str, features_iris, response_iris, true);

boostSvmModel_3.predict(IrisTestRec, features_iris, prediction);
// out
// Boost SVM predict on single Iris:
// [1]

boostSvmModel_3.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost SVM predict on multiple Iris:
// [1, 1, 0]
```
 
 

SVM with specialized metaparams:
```cpp
auto svmModel_4 = metric::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
auto boostSvmModel_4 = metric::Boosting<IrisRec, metric::edmSVM<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel_4);
boostSvmModel_4.train(iris_str, features_iris, response_iris, true);

boostSvmModel_4.predict(IrisTestRec, features_iris, prediction);
// out
// Boost specialized SVM predict on single Iris:
// [1]

boostSvmModel_4.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost specialized SVM predict on multiple Iris:
// [1, 1, 0]
```

###### Bagging

Vector with defined models
```cpp
using WeakLrnVariant = std::variant<metric::edmSVM<IrisRec>, metric::edmClassifier<IrisRec, CSVM> >;
std::vector<WeakLrnVariant> models_1 = {};
WeakLrnVariant svmModel_5 = metric::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
WeakLrnVariant svmModel_6 = metric::edmClassifier<IrisRec, CSVM>();
models_1.push_back(svmModel_5);
models_1.push_back(svmModel_6);
```

Bagging on both specialized and default SVM
```cpp
auto baggingSVMmodel_1 = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
baggingSVMmodel_1.train(iris_str, features_iris, response_iris, true);

baggingSVMmodel_1.predict(IrisTestRec, features_iris, prediction);
// out
// Bagging SVM predict on single Iris:
// [1]

baggingSVMmodel_1.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Bagging SVM predict on multiple Iris:
// [1, 1, 0]
```

Bagging on both specialized and default SVM with deque
```cpp
auto baggingSVMmodel_2 = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
baggingSVMmodel_2.train(iris_strD, features_iris, response_iris, true);

baggingSVMmodel_2.predict(IrisTestRecD, features_iris, prediction);
// out
// Bagging SVM predict on single deque Iris:
// [1]

baggingSVMmodel_2.predict(IrisTestMultipleRecD, features_iris, prediction);
// out
// Bagging SVM predict on multiple deque Iris:
// [1, 1, 0]
```

---

#### C4.5


#### *First example*

Suppose we have set of payments data, where each payment is set of ints:
```cpp
using Record = std::vector<int>;

std::vector<Record> payments = {
	{0,3,5,0},
	{1,4,5,0},
	{2,5,2,1},
	{3,6,2,1}
};
```

Then we defined features and responce (first three elements is features, last one is a label):

```cpp
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
```

And we can define and train C4.5 model:

```cpp
metric::edmClassifier<Record, CC45> c45Model_1 = metric::edmClassifier<Record, CC45>();
c45Model_1.train(payments, features, response);
```

Once model will been trained we can make predict on a test sample:

```cpp
std::vector<Record> test_sample = {
	{0,3,5,0},
	{3,6,2,1}
};

c45Model_1.predict(test_sample, features, prediction);
// out
// C4.5 prediction:
// [0, 1]
```

#### *Iris example*

We can run C4.5 on famous Iris dataset.

Here is our loaded dataset:

```cpp
using IrisRec = std::vector<std::string>;

std::vector<IrisRec> iris_str = read_csv<std::vector<IrisRec>>("./assets/iris.csv");
std::deque<IrisRec> iris_strD = read_csv<std::deque<IrisRec>>("./assets/iris.csv");

iris_str.erase(iris_str.begin()); // remove headers
iris_strD.erase(iris_strD.begin()); // remove headers
```

Here will be our test samples, one is a single sample and another is multiple samples:

```cpp
std::vector<IrisRec> IrisTestRec = { iris_str[5] }; // 1
std::deque<IrisRec> IrisTestRecD = { iris_strD[5] }; // 1

std::vector<IrisRec> IrisTestMultipleRec = { iris_str[5], iris_str[8], iris_str[112] }; // 1, 1, 0
std::deque<IrisRec> IrisTestMultipleRecD = { iris_str[5], iris_str[8], iris_str[112] }; // 1, 1, 0
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

###### Simple model

So, we are ready to define and train simple SVM model: 
```cpp
auto c45Model_2 = metric::edmClassifier<IrisRec, libedm::CC45>();
c45Model_2.train(iris_str, features_iris, response_iris);

c45Model_2.predict(IrisTestRec, features_iris, prediction);
// out
// C4.5 prediction on single Iris:
// [1]

c45Model_2.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// C4.5 prediction on multiple Iris:
// [1, 1, 0]
```

###### Boosting

C4.5 with default metaparams
```cpp
auto boostC45Model_2 = metric::Boosting<IrisRec, metric::edmClassifier<IrisRec, CC45>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model_2);
boostC45Model_2.train(iris_str, features_iris, response_iris, true);

boostC45Model_2.predict(IrisTestRec, features_iris, prediction);
// out
// Boost C4.5 predict on single Iris:
// [1]

boostC45Model_2.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost C4.5 predict on multiple Iris:
// [1, 1, 0]
```

C4.5 with specialized metaparams
```cpp
auto c45Model_3 = metric::edmC45<IrisRec>(2, 1e-3, 0.25, true);
auto boostC45Model_3 = metric::Boosting<IrisRec, metric::edmC45<IrisRec>, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, c45Model_3);
boostC45Model_3.train(iris_str, features_iris, response_iris, true);

boostC45Model_3.predict(IrisTestRec, features_iris, prediction);
// out
// Boost specialized C4.5 predict on single Iris:
// [1]

boostC45Model_3.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost specialized C4.5 predict on multiple Iris:
// [1, 1, 0]
```

###### Bagging

Vector with defined models
```cpp
using WeakLrnVariant = std::variant<metric::edmC45<IrisRec>, metric::edmClassifier<IrisRec, CC45> >;
std::vector<WeakLrnVariant> models_1 = {};
WeakLrnVariant c45Model_4 = metric::edmC45<IrisRec>(2, 1e-3, 0.25, true);
WeakLrnVariant c45Model_5 = metric::edmClassifier<IrisRec, CC45>();
models_1.push_back(c45Model_4);
models_1.push_back(c45Model_5);
```

Bagging on both specialized and default C4.5
```cpp
auto baggingC45model_1 = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
baggingC45model_1.train(iris_str, features_iris, response_iris, true);

baggingC45model_1.predict(IrisTestRec, features_iris, prediction);
// out
// Bagging C4.5 predict on single Iris:
// [1]

baggingC45model_1.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Bagging C4.5 predict on multiple Iris:
// [1, 1, 0]
```

Bagging on both specialized and default C4.5 with deque
```cpp
auto baggingC45model_2 = metric::Bagging<IrisRec, WeakLrnVariant, metric::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, { 0.3, 0.7 }, models_1); // 30% of first weak learner type, 70% of second
baggingC45model_2.train(iris_strD, features_iris, response_iris, true);

baggingC45model_2.predict(IrisTestRecD, features_iris, prediction);
// out
// Bagging C4.5 predict on single deque Iris:
// [1]

baggingC45model_2.predict(IrisTestMultipleRecD, features_iris, prediction);
// out
// Bagging C4.5 predict on multiple deque Iris:
// [1, 1, 0]
```

---

#### Metric Desicion Tree

Here is our dataset:

```cpp
typedef std::variant<double, std::vector<double>, std::vector<std::vector<double>>, std::string> V; // field type
typedef std::vector<V> Record;

std::vector<std::vector<double>> img1 = { // needs to be larger than blur kernel size coded intarnally as 11
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
std::vector<std::vector<double>> img2 = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

std::vector<Record> selection = {
    {V((double)2),
        V(std::vector<double>({1, 2})),
        V(std::vector<double>({0, 1, 1, 1, 1, 1, 2, 3})),
        V(img1),
        V(""),
        V((double)1)
    },
    {V((double)2),
        V(std::vector<double>({1, 5})),
        V(std::vector<double>({1, 1, 1, 1, 1, 2, 3, 4})),
        V(img2),
        V("A"),
        V((double)1)
    },
    {V((double)1),
        V(std::vector<double>({4, 5})),
        V(std::vector<double>({2, 2, 2, 1, 1, 2, 0, 0})),
        V(img2),
        V("AA"),
        V((double)2)
    },
    {V((double)2),
        V(std::vector<double>({1, 2})),
        V(std::vector<double>({3, 3, 2, 2, 1, 1, 0, 0})),
        V(img1),
        V("AAA"),
        V((double)1)
    },
    {V((double)2),
        V(std::vector<double>({5})),
        V(std::vector<double>({4, 3, 2, 1, 0, 0, 0, 0})),
        V(img1),
        V("AAAA"),
        V((double)1)
    },
    {V((double)2),
        V(std::vector<double>({1, 4, 5})),
        V(std::vector<double>({4, 3, 2, 1, 0, 0, 0, 0})),
        V(img2),
        V("BAAA"),
        V((double)1)
    },
    {V((double)1),
        V(std::vector<double>({1, 2, 3, 4})),
        V(std::vector<double>({5, 3, 2, 1, 0, 0, 0, 0})),
        V(img2),
        V("BBAA"),
        V((double)3)
    },
    {V((double)1),
        V(std::vector<double>({1})),
        V(std::vector<double>({4, 6, 2, 2, 1, 1, 0, 0})),
        V(img1),
        V("BBA"),
        V((double)1)
    },
    {V((double)2),
        V(std::vector<double>({4, 5})),
        V(std::vector<double>({3, 7, 2, 1, 0, 0, 0, 0})),
        V(img2),
        V("BB"),
        V((double)1)
    },
    {V((double)2),
        V(std::vector<double>({1, 2, 4, 5})),
        V(std::vector<double>({2, 5, 1, 1, 0, 0, 1, 2})),
        V(img1),
        V("B"),
        V((double)1)
    }
};
```

Then we should define features and responce (first except last elements is features, last one is a label):


```cpp

// vector of accessors for field 0
auto field0accessors = [] (const Record & r) 
{
    return std::get<double>(r[0]);
};

// vector of accessors for field 1
auto field1accessors = [] (const Record & r) 
{
    std::vector<double> v(std::get<std::vector<double>>(r[1]));
    v.resize(4);
    return v;
};
	
// vector of accessors for field 2
auto field2accessors = [] (const Record & r) 
{
    std::vector<double> v(std::get<std::vector<double>>(r[2]));
    v.resize(8);
    return v;
};

// vector of accessors for field 3
auto field3accessors = [] (const Record & r) 
{
    return std::get<std::vector<std::vector<double>>>(r[3]);
};

// vector of accessors for field 4
auto field4accessors = [] (const Record & r) 
{
    return std::get<std::string>(r[4]);
};


// label accessor (for single record)
std::function<int(Record)> response = [](const Record & r)
{
    return (int)std::abs(std::get<double>(r[5]));
};
```

Then we can define dimensions:

```cpp
// build dimension and Dimension objects

typedef double InternalType;
namespace md = metric::distance;

// features
using a0_type = decltype(field0accessors);
using a1_type = decltype(field1accessors);
using a2_type = decltype(field2accessors);
using a3_type = decltype(field3accessors);
using a4_type = decltype(field4accessors);

auto dim0 = metric::make_dimension(md::Euclidian<InternalType>(), field0accessors);
auto dim1 = metric::make_dimension(md::Manhatten<InternalType>(), field1accessors);
auto dim2 = metric::make_dimension(md::P_norm<InternalType>(), field2accessors);
auto dim3 = metric::make_dimension(md::Euclidian_thresholded<InternalType>(), field2accessors);
auto dim4 = metric::make_dimension(md::Cosine<InternalType>(), field2accessors);
auto dim5 = metric::make_dimension(md::SSIM<std::vector<InternalType>>(), field3accessors);
auto dim6 = metric::make_dimension(md::TWED<InternalType>(), field2accessors);
auto dim7 = metric::make_dimension(md::Edit<char>(), field4accessors);
auto dim10 = metric::make_dimension(md::EMD<InternalType>(8,8), field2accessors);

typedef  std::variant<
    metric::Dimension<Record, metric::distance::Euclidian<InternalType>, a0_type>,
    metric::Dimension<Record, metric::distance::Manhatten<InternalType>, a1_type>,
    metric::Dimension<Record, metric::distance::P_norm<InternalType>, a2_type>,
    metric::Dimension<Record, metric::distance::Euclidian_thresholded<InternalType>, a2_type>,
    metric::Dimension<Record, metric::distance::Cosine<InternalType>, a2_type>,
    metric::Dimension<Record, metric::distance::SSIM<std::vector<InternalType>> ,a3_type>,
    metric::Dimension<Record, metric::distance::TWED<InternalType>, a2_type>,
    metric::Dimension<Record, metric::distance::EMD<InternalType>, a2_type>, // matrix C is temporary created inside functor
    metric::Dimension<Record, metric::distance::Edit<std::string::value_type>, a4_type>
    > VariantType;
	
std::vector<VariantType> dims = {dim0, dim1, dim2, dim3, dim4, dim5, dim6, dim7, dim10};
```

And finally we can make a model:


```cpp
std::vector<Record> test_sample = { selection[0], selection[2], selection[6] };
std::vector<int> prediction;

auto model = metric::MetricDT<Record>();
model.train(selection, dims, response);

model.predict(test_sample, dims, prediction);
// out
// Metric Desicion Tree prediction:
// [1, 1, 3]
```

We can calculate distances separately.

- Edit distance
```cpp
metric::distance::Edit<char> edit_functor;
auto edit_dist = edit_functor("AAAB", "AAC");
// out
// Edit distance: 
// 2
```

- SSIM distance
```cpp
metric::distance::SSIM<std::vector<double>> SSIM_functor;
auto SSIM_dist = SSIM_functor(img1, img2);
// out
// SSIM distance: 
// 0.0392542
```

---

#### SOM
---

#### PCAnet
---

#### ESN
---

## Run
*You need STL and C++14 support to compile.*

METRIC | space works headonly. Just include the header into your project.

#### Using CMake

_Windows_

- First of all you need to install [Intel MKL](https://software.intel.com/en-us/mkl)
- Then copy `mkl` (usually from `C:\Program Files (x86)\IntelSWTools\compilers_and_libraries\windows`) folder to the root of the project. 
- Then you can run cmake to create project (links to `mkl` already inside `CMakeLists.txt`)
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

#### Directly, using compiler

For example, include this in your program, f. e. named as `main.cpp`:
```cpp
#include "metric_mapping.hpp"
```

and compile

```bash
$ clang++ ./main.cpp -std=c++14
```