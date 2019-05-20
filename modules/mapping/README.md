# METRIC | SPACE

## Overview


## Examples

##### Quantized mapper (encoder only) 



1. No information
    - kmedoids (with kmeans as specialization) | based on d-full matrices
    - Dbscan | based on d-full matrices
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

#### K-medoids
---

#### K-means

```cpp
std::vector<std::vector<float>> data{
		   {0, 0, 0, 0, 0},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000} };

auto[assignments, means, counts] = clustering::kmeans(data, 4); // clusters the data in 4 groups.
```

---

#### Dbscan

```cpp
std::vector<std::vector<float>> data{
		   {0, 0, 0, 0, 0},
		   {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
		   {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
		   {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
		   {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
		   {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000} };

auto[assignments, seeds, counts] = clustering::dbscan(data, (float) 64.0, 1); 
```

---

#### Affinity propagation
---

#### Hierarchical Clustering
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