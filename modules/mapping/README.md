# METRIC | SPACE

## Overview


## Examples

##### Quantized mapper (encoder only) 



1. No information
    - kmedoids (with kmeans as specialization) | based on d-full matrices
    - DBSCAN | based on d-full matrices
    - Affinity propagation | based on d-full matrices
    - Hierarchical Clustering (t.b.d., should be easy) | based on d-minimal tree


2. Few constrains
    - Laplacian optimization (spectral clustering) | based on d-full matrices
    - Aggregate | based on d-minimal tree


3. Training data
    - Bagging
    - Boosting
    - Metric Decision Tree
    - SVM
    - C4.5


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
auto[assignments, seeds, counts] = clustering::kmedoids(data, 4);
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
auto[assignments, means, counts] = clustering::kmeans(data, 4); 
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
auto[assignments, seeds, counts] = clustering::dbscan(data, (float) 64.0, 1); 
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
auto[assignments, exemplars, counts] = clustering::affprop(data);
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

#### Bagging
---

#### Boosting
---

#### Metric Decision Tree
---

#### SVM

*First example*

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
metric::classification::edmClassifier<Record, CSVM> svmModel_1 = metric::classification::edmClassifier<Record, CSVM>();
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

*Second example*

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


##### Simple SVM model

So, we are ready to define and train simple SVM model: 
```cpp
auto svmModel_2 = metric::classification::edmClassifier<IrisRec, CSVM>();
svmModel_2.train(iris_str, features_iris, response_iris);
```

Once model will been trained we can make predict on a single test sample:

```cpp
svmModel_2.predict(IrisTestRec, features_iris, prediction);
// out
// SVM prediction on single Iris:
// [1]
```

Or on multiple test samples:

```cpp
svmModel_2.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// SVM prediction on multiple Iris:
// [1, 1, 0]
```
 
 
##### Boost SVM model

On the same Iris dataset we can define and train Boost SVM model:
```cpp
auto svmModel_3 = metric::classification::edmClassifier<IrisRec, CSVM>();
auto boostSvmModel_3 = metric::classification::Boosting<IrisRec, metric::classification::edmClassifier<IrisRec, CSVM>, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel_3);
boostSvmModel_3.train(iris_str, features_iris, response_iris, true);
```

Once model will been trained we can make predict on a single test sample:

```cpp
boostSvmModel_3.predict(IrisTestRec, features_iris, prediction);
// out
// Boost SVM predict on single Iris:
// [1]
```

Or on multiple test samples:

```cpp
boostSvmModel_3.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost SVM predict on multiple Iris:
// [1, 1, 0]
```
 
 
##### Boost specialized SVM model

On the same Iris dataset we can define and train Boost specialized SVM model:
```cpp
auto svmModel_4 = metric::classification::edmSVM<IrisRec>(C_SVC, RBF, 3, 0, 100, 0.001, 1, 0, NULL, NULL, 0.5, 0.1, 1, 0);
auto boostSvmModel_4 = metric::classification::Boosting<IrisRec, metric::classification::edmSVM<IrisRec>, metric::classification::SubsampleRUS<IrisRec> >(10, 0.75, 0.5, svmModel_4);
boostSvmModel_4.train(iris_str, features_iris, response_iris, true);
```

Once model will been trained we can make predict on a single test sample:

```cpp
boostSvmModel_4.predict(IrisTestRec, features_iris, prediction);
// out
// Boost specialized SVM predict on single Iris:
// [1]
```

Or on multiple test samples:

```cpp
boostSvmModel_4.predict(IrisTestMultipleRec, features_iris, prediction);
// out
// Boost specialized SVM predict on multiple Iris:
// [1, 1, 0]
```

---

#### C4.5
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

For example, include this in `main.cpp` (can be found in the examples dir):
```cpp
#include "metric_mapping.cpp"
```

and compile

```bash
$ clang++ ./main.cpp -std=c++14
```