# METRIC | DISTANCE
*A templated, header only C++14 implementation of a Metric Distance Functions.*

## Overview

If we can assign each column a metric, we have won. Then all algorithms run on the table without the need for further restrictions or special features. 

There are a lot of metrics in the world, that can be grouped to:
- **k-related**: number of entries in column is always the same and the metric applied to the corresponding 
entries in the compared records. You can switch the entries order pairwise without changing the resulting metric. 
And metric calculated as costs to adapt one value to the other. It calls a *scale function* in METRIC terms. 
Then all adapt costs reduce to a single metric value. It calls a *reduce function* in METRIC terms.
- **k-structured**: number of entries in column can be various and it has a structure. The metric is calculated 
between all entries with *cost matrix*, that represents entries structure. *Scale function* and *costs matrix* gives 
costs of transforming one record to the other. 
- **k-random**: opposing entries not related, they are just sampled. It's the same as others, but more abstract. 
It's about the costs of transforming one bunch of samples to the other, with pure scale information.

In **METRIC** framework terms a metric is the cost to transform one record to the other record!

And framework thinks about all the metrics as one metric with a *scale function*, a *reduce function* or a *cost matrix*. 

**METRIC** framework have one universal metric factory function. To specify needed metric the user should only specify a 
*scale function*, a *reduce function* or a *cost matrix*. And of course framework provide the common metrics by name.

The framework’s metric factory backes the metric, which is than just a function. 

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
metric::distance::Euclidian<double> euclidianL2Distance;
auto result_1 = euclidianL2Distance(v0, v1);
std::cout << "result: " << result_1 << std::endl;
// out:
// Euclidean (L2) metric
// result: 2
```

- **Euclidean Threshold metric**
``` cpp
metric::distance::Euclidian_thresholded<double> euclidianThresholdDistance(1000.0, 3000.0);
auto result_2 = euclidianThresholdDistance(v0, v1);
std::cout << "result: " << result_2 << std::endl;
// out:
// Euclidean Threshold metric
// result: 1000
```

- **Manhatten/Cityblock (L1) metric**
``` cpp
metric::distance::Manhatten<double> manhattenL1Distance;
auto result_3 = manhattenL1Distance(v0, v1);
std::cout << "result: " << result_3 << std::endl;
// out:
// Manhatten/Cityblock (L1) metric
// result: 4
```

- **Minkowski (L general) metric**
``` cpp
metric::distance::P_norm<double> pNormDistance(2);
auto result_4 = pNormDistance(v0, v1);
std::cout << "result: " << result_4 << std::endl;
// out:
// Minkowski (L general) metric
// result: 2
```

- **Cosine metric**
``` cpp
metric::distance::Cosine<double> cosineDistance;
auto result_5 = cosineDistance(v0, v1);
std::cout << "result: " << result_5 << std::endl;
// out:
// Cosine metric
// result: 0.970143
```
---

### Earth Mover Distance metric

Suppose we have an images as matrices: `img1`, `img2`.

Now we can reshape matrices to vectors:
```cpp
typedef int edm_Type;

size_t im1_R = img1.size() / 6;
size_t im1_C = img1[0].size() / 6;

// serialize_mat2vec
std::vector<edm_Type> i1;
std::vector<edm_Type> i2;

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
auto cost_mat = metric::distance::EMD_details::ground_distance_matrix_of_2dgrid<edm_Type>(im1_C, im1_R);
auto maxCost = metric::distance::EMD_details::max_in_distance_matrix(cost_mat);
```
Then declare EMD (Earth Mover Distance) metric and use it:

```cpp
metric::distance::EMD<edm_Type> distance(cost_mat, maxCost);

auto result = distance(i1, i2);
std::cout << "result: " << result << std::endl;
// out:
// Earth Mover Distance metric
// result: 4531000
```
---

### Edit Distance metric (for strings)

Suppose we have two strings:

```cpp
std::string str1 = "1011001100110011001111111";
std::string str2 = "1000011001100110011011100";
```
We can use Edit Distance metric for compare that strings:

```cpp
metric::distance::Edit<std::string> distance;

auto result = distance(str1, str2);
std::cout << "result: " << result << std::endl;
// out:
// Edit Distance metric
// result: 5
```

---

### Time Warp Elastic Distance metric (for curves)
---


### Structural Similarity metric (for images)
---

## Run
*You need STL and C++14 support to compile.*

METRIC | distance works headonly. Just include the header into your project.

```cpp
#include "metric_distance.cpp"
```

and compile for example with

```bash
$ clang++ ./examples/StandartMetrics_example.cpp -std=c++14
```