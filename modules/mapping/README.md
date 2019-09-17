# METRIC | MAPPING

## Overview


## Examples

##### Quantized mapper (encoder only) 



1. No information
    - K-Medoids (with K-Means as specialization) | based on d-full matrices
    - DBSCAN | based on d-full matrices
    - Affinity propagation | based on d-full matrices
    - Hierarchical Clustering | based on d-minimal tree


2. Few constrains
    - Laplacian optimization (spectral clustering) | based on d-full matrices
    - Aggregate | based on d-minimal tree


3. Training data
    - SVM
    - C4.5


##### Continuous mapper (encoder/decoder)



1. No information
    - SOM (k-related space to k-structured space converter)
    - PCFA (k-structured space to k-related space converter)


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


Suppose we have the following records:
```cpp
std::vector<std::vector<float>> data{
		   {0, 0, 0, 0, 0},
		   {1, 1, 1, 1, 1},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000}
	};
```
And here is example of how can be found clusters using Affinity Propagation:
```cpp	   	 
auto hc = metric::HierarchicalClustering<Record, metric::Euclidian<Record::value_type>>(data, 3);
hc.hierarchical_clustering();

for (size_t i = 0; i < hc.clusters.size(); i++)
{
	std::cout << "cluster #" << i << std::endl;
	for (size_t j = 0; j < hc.clusters[i].data.size(); j++)
	{
		for (size_t k = 0; k < hc.clusters[i].data[j].size(); k++)
		{
			std::cout << hc.clusters[i].data[j][k] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

// out:

// cluster #0
// 2.85493 3.2538 2.50559 68.5184 68.5184
// 2.57927 2.63399 2.46802 61.9026 61.9026
// 1.7412 4.07812 -0.0927036 41.7888 41.7888

// cluster #1
// 0 0 0 0 0
// 1 1 1 1 1
// 7.75309 16.2466 3.03956 186.074 186.074
// 5.81414 8.14015 3.2295 139.539 139.539
```

---

#### Laplacian optimization
---

#### Aggregate
---



#### SVM


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
metric::edmClassifier<Record, CSVM> svmModel = metric::edmClassifier<Record, CSVM>();
svmModel.train(payments, features, response);
```

Once model will been trained we can make predict on a test sample:

```cpp
std::vector<Record> test_sample = {
	{0,3,5,0},
	{3,6,2,1}
};
std::vector<bool> prediction;

svmModel.predict(test_sample, features, prediction);
// out
// prediction:
// [0, 1]
```
 
 

---

#### C4.5

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
metric::edmClassifier<Record, CC45> c45Model = metric::edmClassifier<Record, CC45>();
c45Model.train(payments, features, response);
```

Once model will been trained we can make predict on a test sample:

```cpp
std::vector<Record> test_sample = {
	{0,3,5,0},
	{3,6,2,1}
};

c45Model.predict(test_sample, features, prediction);
// out
// C4.5 prediction:
// [0, 1]
```

---

#### SOM

Suppose we have image `img1` as `std::vector<std::vector<double>>`. 
Then we can create and train SOM (reduce dimensions):

```cpp
using Vector = std::vector<double>;
using Metric = metric::Euclidian<Vector::value_type>;
using Graph = metric::Grid6;

metric::SOM<Vector, Metric, Graph> som_model(6, 5);

unsigned int iterations = 1000;
som_model.train(img1, iterations);
```
After train we can find distances for the record (f.e. row from image matrix) for all SOM nodes:

```cpp
auto dimR = som_model.reduce(img1[0]);
// out
// dimR:
// 510.852 481.27 463.812 474.718 518.213 547.852
// 476.219 445.141 447.774 491.266 553.524 597.049
// 444.867 435.58 426.204 466.432 528.757 599.06
// 404.8 414.577 445.764 512.059 582.815 616.019
// 415.284 392.53 416.091 464.418 543.318 588.668
```
Or find the best matching unit from reduced space:
```cpp
auto bmu = som_model.BMU(img1[0]);
// out
// bmu:
// 25
```

---

#### PCFA

First of all let's create two sine datasets - for train and test:

```cpp
size_t n_freq_steps = 10;
size_t n_slices_per_step = 100;
size_t waveform_length = 64;

blaze::DynamicMatrix<double, blaze::columnMajor>  SlicesSine(waveform_length, n_freq_steps*n_slices_per_step, 0.0);
blaze::DynamicMatrix<double, blaze::columnMajor>  TestSlicesSine(waveform_length, n_freq_steps, 0.0);

double frequenz; // based on original test case code
double phase = 0;
double delta_T = 0.05;

// sine generator
size_t idx = 0;
for (size_t ii = 1; ii <= n_freq_steps; ii++) // frequency change steps
{
	frequenz = double(ii) / double(n_freq_steps);
	for (size_t i = 0; i < n_slices_per_step; ++i) // slices with same freq and random phases (within each freq step)
	{
		phase = (double)rand() / RAND_MAX * 0.9 + 0.1;
		for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
		{
			SlicesSine(t, idx) = sin(2 * M_PI * (frequenz * double(t) * delta_T + phase));
		}
		idx++;
	}
}

idx = 0;
for (size_t i = 1; i <= n_freq_steps; i++) // frequency steps
{
	frequenz = double(i) / double(n_freq_steps);
	phase = 0; //(double)rand()/RAND_MAX; // 0;
	for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
	{
		TestSlicesSine(t, idx) = sin(2 * M_PI * (frequenz * double(t) * delta_T + phase));
	}
	idx++;
}
```

And now we can create PCFA model:

```cpp
auto direct_sine = metric::PCFA<double>(SlicesSine, 8);
```

With PCFA model we are ready to encode `TestSlicesSine`:
```cpp
auto direct_compressed_sine = direct_sine.encode(TestSlicesSine);
```

And decode back:
```cpp
auto direct_restored_sine = direct_sine.decode(direct_compressed_sine);
```

---

#### ESN
Suppose we have train, target and test datasets:

```cpp
blaze::DynamicMatrix<double, blaze::rowMajor>  SlicesR {
    {1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
    {0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
    {0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   },
    {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.25, 0   , 0   , 0   },
    {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75},
    {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25}
};

blaze::DynamicMatrix<double, blaze::rowMajor> TargetR {
    {-0.45, -0.4, -0.35, -0.3, -0.25, -0.2, -0.15, -0.1, -0.05, 0   , 0.05, 0.1, 0.15 , 0.2 , 0.25 , 0.3 , 0.35 , 0.4 },
    {0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25},
};
// first line (position of peak) is easy to predict, second is much harder. ESN predicts it better in no-echo mode

blaze::DynamicMatrix<double, blaze::rowMajor>  SlicesTestR {
    {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25},
    {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75},
    {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.25, 0   , 0   , 0   },
    {0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   },
    {0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
    {1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   }
};
```

Then we can create and train ESN model:
```cpp
auto esn = metric::ESN(500, 4, 0.99, 0.5, 5, 0.9); // echo
esn.train(SlicesR, TargetR);
```

Ad with trained model we can make prediction:
```cpp
auto predictions = esn.predict(SlicesTestR);
// out
// predictions:
// (     0.332319     0.330211     0.305872      0.26673     0.228705     0.190869     0.151045     0.107785    0.0643527    0.0255038   -0.0254055   -0.0916294    -0.110181 )
// (     0.397573     0.417724     0.409599     0.415551     0.315736     0.217372     0.162721     0.160267    0.0886803   -0.0269741   -0.0692777   -0.0419959  -0.00955842 )
```

---
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

---

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