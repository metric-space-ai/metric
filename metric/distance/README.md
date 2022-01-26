# Distances

A distance function d(a,b) must fullfill four requriements to be metric.

1. d(a, b) ≥ 0 (non-negativity)
2. d(a, b) = 0 if and only if a = b (identity of indiscernibles)
3. d(a, b) = d(b, a) (symmetry)
4. d(a, b) ≤ d(a, c) + d(c, b) (triangle inequality)

When 2. holds only d(a, b) = 0 but not the identity of indisccernibles it is a semi-metric or pseudo-metric. When 4. is
not valid then it is only a distance but not a metric. Only "real" metrics and pseudo-metrics are provided in the
library. Pseudo-metrics are used for random data, because there is not deterministic indentity In principle, it is
possible to use an arbitrary distance definition for some of the algorihm, but it is conceptual very invalid to do so
and no garantees can be given.

# Random metrics

## The cdf family

*cdf = cummulative distribution function*

### Wasserstein (L1 norm on cdf)

This is the equivalent for the Manhatten metric for random sampled data, also know as Wasserstein distance.

### Cramér-von Mises (L2 norm on cdf)

This is the equivalent of the Euclidean metric for random sampled data.

### Kolmogorov-Smirnov (L∞ norm on cdf)

This is the equilvalent for the Chebychev metric for random sampled data.

## The entropy family

### VMixing

This a measure of difference between entropies of datasets and entropy of concatenated dataset. Not a true metric.

# Related metrics

## The norm family

Based on the concept of norm induced metric spaces, one can derive serveral distance functions.

### Euclidean (L2 norm, geometric difference)

In the two-dimensional plane or in three-dimensional space, the Euclidean distance d(a,b) corresponds to the distance
between the points a and b, that can be computed by the law of pythagoras. In the more general case of n-dimensions, the
euclidean distance is defined by the L2 Norm of the difference vector. If the points a and b are defined by the
coordinates a=(a_1,...,a_n) and b=(b_1,...,b_n), then the equations leads to the distance:
d(a,b)=sqrt( (a_1 - b_1)^2 + ... + (a_n - b_n)^2 )
https://en.wikipedia.org/wiki/Euclidean_distance

Example

```cpp
std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };

metric::Euclidean<std::vector<double>> euclideanDistance;
auto result = euclideanDistance(v0, v1);
// result = 2

```

### Manhatten (L1 norm, absolute difference)

The Manhatten metric is also known as rectilinear distance, L1 distance, L1 norm, snake distance, city block distance,
taxicab distance or Manhattan length. It is the sum of the absolute difference of the vector entries. d(a,b)= abs(a_1 -
b_1) + ... + abs(a_n - b_n) )
https://en.wikipedia.org/wiki/Taxicab_geometry

### Chebychev (L∞ norm, maximum difference)

The Chebyshev distance (also known as Tchebychev distance, maximum metric, or L∞ metric is defined as the distance
between two vectors, that is the greatest of their differences along any coordinate dimension. d(a,b)= max(abs(a_1 -
b_1), ..., abs(a_n - b_n) )
https://en.wikipedia.org/wiki/Chebyshev_distance

### Minkowski (Lp norm)

The Minkowski distance can be considered as a generalization of the Euclidean, Manhattan and Chebychev distance. The
parameter p defines the metric. p=Inf is Chebychev metric, p=1.0 in the Manhatten metric and p=2.0 in the Euclidean
metric. The parameter could be freely choosen between [0.0 ... Inf]

d(a,b)=( (a_1 - b_1)^p + ... + (a_n - b_n)^p )^(1/p)

https://en.wikipedia.org/wiki/Minkowski_distance

## The absolute difference family

Several distance in the literatue are based on the Manhatten metric, more precisely on the idea of the absolute
difference vector, but utilizes some kind of build-in normalization instead of using pre-scaled values per
standardization or after scaled distances by thresholding.

### Canberra (metric)

https://en.wikipedia.org/wiki/Canberra_distance
similiar to Sørensen (Bray-Curtis) distance, but it's known to be very sensitive to small changes near zero. Instead of
Sørensen distance, it's a metric.

In mathematics, also is known as the `Wasserstein` metric.

Informally, if the distributions are interpreted as two different ways of piling up a certain amount of dirt over the
region `D`, the `EMD` is the minimum cost of turning one pile into the other; where the cost is assumed to be amount of
dirt moved times the distance by which it is moved.

Before creating EMD object you need to construct ground distance matrix. Where each value is a distance for move a
single pile from one point of matrix to another. You can do it by one of the predefined functions or by self:

```cpp
auto ground_distance_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<T, Metric>(columns, rows);
auto ground_distance_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<T, Metric>(2d_vector);
```

_Note: For images you should serialize the pixel's 2D array in a vector and compute the ground distance matrix of the
original picture sized grid._

***For example***, suppose we have an images as matrices: `img1`, `img2`.

Now we should reshape matrices to vectors:

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

### Inner Product (metric!)

First we should calculate a ground distance matrix:

```cpp
auto ground_distance_mat = metric::EMD_details::ground_distance_matrix_of_2dgrid<emd_Type>(im1_C, im1_R);
auto max_distance = metric::EMD_details::max_in_distance_matrix(ground_distance_mat);
```

Then declare EMD (Earth Mover Distance) metric and use it:

```cpp
metric::EMD<emd_Type> distance(ground_distance_mat, max_distance);




# Structured metrics
## The Edit family
The idea of an edit distance is to transform one record to another by basic edit operations. Each basic operation is assign to a cost, for example to shift one entry to the next index. The distance is the minimum possible sum of costs and therefor realted to the optimal transforming path. Here, an optimization problem is solved, for example by dynamical programming.

### Levenshtein
Also known as edit distance, can be used for character arrays aka strings.
### Longest Common Subsequence (LCS)
### Dynamic Time Warping (DTW)
### Time Warp Edit Distance (TWED)
Costs are defined for an y-value and x-value edit, often the time. Therefor it can be used to compare two curves that are not in phase.


## The Earth Mover's Distance
The Earth Mover's distance is some kind of universial metric. the entries of the records are modelled as nodes of a graph. the distance is the minimal cost to flow mass through the edges of the graph. The costs are configured by a ground distance matrix, which represents the basic costs between the nodes, the edge values of the graph.

ground_distance_matrix_for_1dgrid
ground_distance_matrix_for_2dgrid(w,h)

ground_distance_matrix_for_Image(c,w,h)
ground_distance_matrix_for_hog(w,...)


# metric for metric spaces
## Riemannian
A metric space can be represented through a pairwise distance matrix. This matrix is symmetric and positive definite. One can measure the distance between two metric spaces by the Riemannian metric.

coming soon

## Graph Edit
coming soon

## Hausdorf 
coming soon

## Tree Edit
comming soon


# Standardization of a metric (pre-processing)
Often, some form of standardization of the input values can help to balance out the contributions of different scales in the dimensions of the record. The conventional way to do this is to transform the values so they all have the same variance of 1 and at the same time to centre the values at their means.

To etimate the mean and the standard deviation the metric and build with a referenz data set, which provides samples of the expected data. The mean and standard deviation are computed from that set and stored inside the distance function. The transformation of the value it done on the fly, when the metric is called.
standardized value = (original value – mean) / standard deviation

The Standardization works as wrapper for an abritrary metric.

Example
```cpp
std::vector<double> reference_sample = {
        { 0, 1, 1, 0.01, 1, 1, 200, 3 },
        { 1, 1, 1, 0.01, 1, 2, 300, 4 },
        { 2, 2, 2, 0.01, 1, 2, 0, 0 },
        { 3, 3, 2, 0.02, 1, 1, 0, 0 },
        { 4, 3, 2, 0.01, 0, 0, 0, 0 },
        { 5, 3, 2, 0.01, 0, 0, 0, 0 },
        { 4, 6, 2, 0.02, 1, 1, 0, 0 }};

metric::standardized<Euclidean<double>> s_euclidean_distance(reference_sample);

double result = s_euclidean_distance(A[0], A[4]);
```

However, the results are not necessary better by standardization, because outliners could have a huge impact and lead to
a collapse of the variance.

# Thresholdding of a metric (post-processing)

For high dimensional vectors, it could make sense to non-linear deform the resulting distance of a metric by hard or
soft clipping the distance value. Additionally the distance value can be scaled below the threhshold to clipp faster or
slower againt the threshold. Clipping leads to better computation times, when used for the EMD ground distance for
example.

Example

```cpp
metric::hard_clipped<metric::Euclidean<double>> euclidean_thresholded(8.0/*,1.0*/);
auto result1 = euclidean_thresholded(v0, v1);
```

Instead of hard clipping the values, you can use a continous saturation, that is modeled with an exponential function.

```cpp
metric::soft_clipped<metric::Euclidean<double>> euclidean_scaled_and_limited(8.0 ,0.8, 2.0);
auto result2 = euclidean_scaled_and_limited(v0, v1);
```

The threshold [0.01..0.99] is realated to the max_distance value and defines the percentage level where the saturation
starts. Below that point, the value is scaled linearly with the scal factor. The scal factor must be positve. values
greater one will shorten the range until saturation takes place and a value less one will longer the range until the
limitting starts.

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

metric::Kohonen<double, std::vector<double>> distance(train_dataset, grid_w, grid_h);

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
	
metric::Kohonen<double, Vector, Graph, Metric> distance(som_model);

auto result = distance(train_dataset[0], train_dataset[1]);
std::cout << "result: " << result << std::endl;
// out:
// Kohonen metric
// result: 772.109
```

*For a full example and more details see `examples/distance_examples/Kohonen_example.cpp`*



---

### Entropy and VMixing

Suppose we have some data:

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

For the same data we can calculate "variation for mixing": or, in short words, compare entopies of each dataset to
entropy of concatenated datset:

```cpp
auto vm = VMixing(); // default parameters
auto result = metric::vm(v1, v2);
std::cout << "result: " << result << std::endl;
```

Again we can specify distance metric:

```cpp
auto vm = metric::VMixing_simple<void, metric::Manhatten<double>>(metric::Manhatten<double>())
auto result = vm(v1, v2);
std::cout << "VMixed Manhatten result: " << result << std::endl;
std::cout << "result: " << result << std::endl;
```

*For a full example and more details see `examples/distance_examples/entropy_example.cpp`
, `tests/distance_tests/entropy_vmixing_tests.cpp`*

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
