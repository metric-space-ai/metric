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

#### Dbscan
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