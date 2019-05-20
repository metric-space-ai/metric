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