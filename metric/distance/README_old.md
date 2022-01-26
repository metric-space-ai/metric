# METRIC | DISTANCE

*A templated, header only C++17 implementation of a Metric Distance Functions.*

## Overview

METRIC | DISTANCE provide a extensive collection of metrics, including factory functions for configuring complex
metrics.  
They are organized into severals levels of complexity and aprio knowledge about the data. Basically the user give a
priori information, how the data should be connected for reason, like a picuture is a 2d array of pixels. A metric type
is basically a function, which compares two samples of data and gives back the numeric distance between them.

If we can assign each feature from the data a metric, we have won. Then all algorithms run on the dataset without the
need for further restrictions or special features.

There are a lot of metrics in the world, that can be grouped to:

- **k-related**: number of entries in column is always the same and the metric applied to the corresponding entries in
  the compared records. You can switch the entries order pairwise without changing the resulting metric. And metric
  calculated as costs to adapt one value to the other. It calls a *scale function* in METRIC terms. Then all adapt costs
  reduce to a single metric value. It calls a *reduce function* in METRIC terms.
- **k-structured**: number of entries in column can be various and it has a structure. The metric is calculated between
  all entries with *cost matrix*, that represents entries structure. *Scale function* and *costs matrix* gives costs of
  transforming one record to the other.
- **k-random**: opposing entries not related, they are just sampled. It's the same as others, but more abstract. It's
  about the costs of transforming one bunch of samples to the other, with pure scale information.

In **METRIC** framework terms a metric is the cost to transform one record to the other record!

And framework thinks about all the metrics as one metric with a *scale function*, a *reduce function* or a *cost matrix*
.

**METRIC** framework have one universal metric factory function. To specify needed metric the user should only specify a
*scale function*, a *reduce function* or a *cost matrix*. And of course framework provide the common metrics by name.

The framework's metric factory backes the metric, which is than just a function.

## Examples

### Standard metrics

Suppose we have a table with two records, where each columns is simple one-dimensional entry of double value:

```cpp
std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
```

Using **METRIC** framework we can calculate a set of standard metrics for this records.

- **Euclidean (L2) metric**

``` cpp
metric::Euclidean<double> euclideanL2Distance;
auto result_1 = euclideanL2Distance(v0, v1);
std::cout << "result: " << result_1 << std::endl;
// out:
// Euclidean (L2) metric
// result: 2
```

- **Euclidean Threshold metric**

``` cpp
metric::Euclidean_thresholded<double> euclideanThresholdDistance(1000.0, 3000.0);
auto result_2 = euclideanThresholdDistance(v0, v1);
std::cout << "result: " << result_2 << std::endl;
// out:
// Euclidean Threshold metric
// result: 1000
```

- **Manhatten/Cityblock (L1) metric**

``` cpp
metric::Manhatten<double> manhattenL1Distance;
auto result_3 = manhattenL1Distance(v0, v1);
std::cout << "result: " << result_3 << std::endl;
// out:
// Manhatten/Cityblock (L1) metric
// result: 4
```

- **Minkowski (L general) metric**

``` cpp
metric::P_norm<double> pNormDistance(2);
auto result_4 = pNormDistance(v0, v1);
std::cout << "result: " << result_4 << std::endl;
// out:
// Minkowski (L general) metric
// result: 2
```

- **Cosine metric**

``` cpp
metric::Cosine<double> cosineDistance;
auto result_5 = cosineDistance(v0, v1);
std::cout << "result: " << result_5 << std::endl;
// out:
// Cosine metric
// result: 0.970143
```

*For a full example and more details see `examples/distance_examples/standart_distances_example.cpp`*

---

### Earth Mover Distance metric

Suppose we have an images as matrices: `img1`, `img2`.

Now we can reshape matrices to vectors:

```cpp
typedef int emd_Type;

size_t im1_R = img1.size();
size_t im1_C = img1[0].size();

// serialize_mat2vec
std::vector<emd_Type> i1;
std::vector<emd_Type> i2;

for (size_t i = 0; i < im1_R; ++i)
{
	for (size_t j = 0; j < im1_C; ++j)
	{
		i1.push_back(img1[i][j]);
		i2.push_back(img2[i][j]);
	}
}
```

And now we can compare two vectors using Earth Mover Distance.

First we should calculate a cost matrix:

```cpp
auto cost_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<emd_Type>(im1_C, im1_R);
auto maxCost = metric::EMD_details::max_in_distance_matrix(cost_mat);
```

Then declare EMD (Earth Mover Distance) metric and use it:

```cpp
metric::EMD<emd_Type> distance(cost_mat, maxCost);

auto result = distance(i1, i2);
std::cout << "result: " << result << std::endl;
// out:
// Earth Mover Distance metric
// result: 4531000
```

*For a full example and more details see `examples/distance_examples/earth_mover_distance_example.cpp`
and `examples/distance_examples/earth_mover_distance_2_example.cpp`*

---

### Edit Distance metric (for strings)

Suppose we have two strings:

```cpp
std::string str1 = "1011001100110011001111111";
std::string str2 = "1000011001100110011011100";
```

We can use Edit Distance metric for compare that strings:

```cpp
metric::Edit<std::string> distance;

auto result = distance(str1, str2);
std::cout << "result: " << result << std::endl;
// out:
// Edit Distance metric
// result: 5
```

*For a full example and more details see `examples/distance_examples/edit_distance_example.cpp`*

---

### Time Warp Elastic Distance metric (for curves, series or time-series)

Suppose we have two series:

```cpp
std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
```

We can use Time Warp Elastic Distance metric for compare that series:

```cpp
metric::TWED<double> distance;

auto result = distance(v0, v1);
std::cout << "result: " << result << std::endl;
// out:
// Time Warp Elastic Distance metric
// result: 7
```

*For a full example and more details see `examples/distance_examples/time_warp_elastic_distance_example.cpp`*

---

### Structural Similarity metric (for images)

Suppose we have two images `img1` and `img2` as `std::vector<std::vector<int>>`. Then we can use Structural Similarity
metric for compare that images:

```cpp
metric::SSIM<double, std::vector<int>> distance;

auto result = distance(img1, img2);
std::cout << "result: " << result << std::endl;
// out:
// Structural Similarity metric
// result: 0.0907458
```

*For a full example and more details see `examples/distance_examples/structural_similarity_example.cpp`*

---

### Sorensen

Suppose we have a set of values:

```cpp
typedef double V_type;

V_type vt0 = 0;
V_type vt1 = 1;
V_type vt2 = 2;
V_type vt3 = 3;
```

Then we can calculate Sorensen distances various vector types.

`STL`:

```cpp
std::vector<V_type> obj1 = {vt0, vt1, vt2, vt0};
std::vector<V_type> obj2 = {vt0, vt1, vt3};
	
auto result = metric::sorensen(obj1, obj2);
std::cout << "result: " << result << std::endl;
// out:
// Sorensen distance for std::vector
// result: 0.142857
```

`blaze::DynamicVector`:

```cpp
blaze::DynamicVector<V_type> bdv1 {vt0, vt1, vt2, vt0};
blaze::DynamicVector<V_type> bdv2 {vt0, vt1, vt3};
	
auto result = metric::sorensen(bdv1, bdv2);
std::cout << "result: " << result << std::endl;
// out:
// Sorensen distance for blaze::DynamicVector
// result: 0.142857
```

`blaze::StaticVector`:

```cpp
blaze::StaticVector<V_type, 4UL> bsv1 {vt0, vt1, vt2, vt0};
blaze::StaticVector<V_type, 4UL> bsv2 {vt0, vt1, vt3, vt0};
	
auto result = metric::sorensen(bsv1, bsv2);
std::cout << "result: " << result << std::endl;
// out:
// Sorensen distance for blaze::StaticVector
// result: 0.142857
```

`blaze::HybridVector`:

```cpp
blaze::HybridVector<V_type, 4UL> bhv1 {vt0, vt1, vt2, vt0};
blaze::HybridVector<V_type, 4UL> bhv2 {vt0, vt1, vt3, vt0};
	
auto result = metric::sorensen(bhv1, bhv2);
std::cout << "result: " << result << std::endl;
// out:
// Sorensen distance for blaze::HybridVector
// result: 0.142857
```

`blaze::CompressedVector`:

```cpp
blaze::CompressedVector<V_type> bcv1 {vt0, vt1, vt2, vt0};
blaze::CompressedVector<V_type> bcv2 {vt0, vt1, vt3};
	
auto result = metric::sorensen(bcv1, bcv2);
std::cout << "result: " << result << std::endl;
// out:
// Sorensen distance for blaze::CompressedVector
// result: 0.142857
```

*For a full example and more details see `examples/distance_examples/sorensen_distance_example.cpp`*

---

### Kohonen distance

The idea of the Kohonen distance is: to train a SOM on a dataset and then compute the EMD for two records in the Kohonen
space.

Suppose we have a set of values:

```cpp
std::vector<std::vector<double>> train_dataset = {
    { 26.75,	22.15 },
    { 29.8,	22.15 },
    { 31.55,	21.1 },
    { 27.7,	20.85 }
};
```

Then we can create Kohonen distance object and calculate distance. When dataset is passed to the constructor, Kohonen
distance object will train incapsulated SOM on that dataset.

```cpp
int grid_w = 6;
int grid_h = 4;

metric::kohonen_distance<double, std::vector<double>> distance(train_dataset, grid_w, grid_h);

auto result = distance(train_dataset[0], train_dataset[1]);
std::cout << "result: " << result << std::endl;
// out:
// Kohonen metric
// result: 824.567
```

Other way to initialize Kohonen distance object is to pass pretrained SOM object to the constructor:

```cpp	
int grid_w = 6;
int grid_h = 4;
	
using Vector = std::vector<double>;
using Metric = metric::Euclidean<double>;
using Graph = metric::Grid6; 
using Distribution = std::uniform_real_distribution<double>; 

Distribution distr(-1, 1);

metric::SOM<Vector, Graph, Metric> som_model(Graph(grid_w, grid_h), Metric(), 0.8, 0.2, 20, distr);
som_model.train(train_dataset);
	
metric::kohonen_distance<double, Vector, Graph, Metric> distance(som_model);

auto result = distance(train_dataset[0], train_dataset[1]);
std::cout << "result: " << result << std::endl;
// out:
// Kohonen metric
// result: 772.109
```

*For a full example and more details see `examples/distance_examples/kohonen_distance_example.cpp`*



---

### Entropy, Mutual Information and Variation of Information

Suppose we have a some data:

```cpp
std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };
```

Then we can calculate the entropy of the given data:

```cpp
auto estimator = metric::Entropy<std::vector<double>>();
auto result = estimator(v);
std::cout << "result: " << result << std::endl;
// out:
// Entropy using default distance metric
// result: -5.39891
```

Of cause, we can calculate entropy using any distance metric:

```cpp
auto estimator = metric::Entropy<void, metric::Manhatten<double>>();
auto result = estimator(v);
std::cout << "result: " << result << std::endl;
// out:
// Entropy using Manhatten distance metric
// result: 0.132185
```

And now suppose we have two vectors with a data:

```cpp
std::vector<std::vector<double>> v1 = {{5,5}, {2,2}, {3,3}, {5,5}};
std::vector<std::vector<double>> v2 = {{5,5}, {2,2}, {3,3}, {1,1}};
```

Then we can calculate Mutual Information:

```cpp
auto result = metric::mutualInformation(v1, v2);
std::cout << "result: " << result << std::endl;
// out:
// Mutual Information using default distance metric
// result: 1.00612
```

Of cause we can specify distance metric:

```cpp
auto result = metric::mutualInformation(v1, v2, 3, metric::Euclidean<double>());
std::cout << "result: " << result << std::endl;
// out:
// Mutual Information using Euclidean distance metric
// result: 0.797784 
```

For the same data we can calculate Variation of Information:

```cpp
auto result = metric::variationOfInformation(v1, v2);
std::cout << "result: " << result << std::endl;
// out:
// Variation of Information using default distance metric
// result: 0
```

Again we can specify distance metric:

```cpp
auto result = metric::variationOfInformation<std::vector<std::vector<double>>, metric::Manhatten<double>>(v1, v2);
std::cout << "VOI Manhatten result: " << result << std::endl;
std::cout << "result: " << result << std::endl;
// out:
// Variation of Information Information using Manhatten distance metric
// result: 0
```

We can calculate normalized Variation of Information:

```cpp
auto result = metric::variationOfInformation_normalized(v1, v2);
std::cout << "result: " << result << std::endl;
// out:
// normalized Variation of Information
// result: 1.08982
```

Instead function we can use functor for Variation of Information:

```cpp
auto f_voi = metric::VOI<long double>();
auto result = f_voi(v1, v2);
std::cout << "result: " << result << std::endl;
// out:
// Variation of Information as functor
// result: 0
```

Normalized functor:

```cpp
auto f_voi_norm = metric::VOI_normalized<long double>();
auto result = f_voi_norm(v1, v2);
std::cout << "result: " << result << std::endl;
// out:
// normalized Variation of Information as normalized functor
// result: 1.08982
```

*For a full example and more details see `examples/distance_examples/entropy_example.cpp`*

---

## Run

*You need STL and C++17 support to compile.*

METRIC | DISTANCE works headonly. Just include the header into your project.

```cpp
#include "metric/distance.hpp"
```

or directly include one of specified distance from the following:

```cpp
#include "metric/distance/k-related/Standards.hpp"
#include "metric/distance/k-related/L1.hpp"

#include "metric/distance/k-structured/SSIM.hpp"
#include "metric/distance/k-structured/TWED.hpp"
#include "metric/distance/k-structured/EMD.hpp"
#include "metric/distance/k-structured/Edit.hpp"

#include "metric/distance/k-random/VOI.hpp"
```

#### CMake compilation

Inside folder with your cpp file or inside `examples/distance_examples/` run the following commands:

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
$ clang++ ./examples/distance_examples/standart_distances_example.cpp -std=c++17
```
