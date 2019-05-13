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

**Euclidian (L2) Metric**
``` cpp
metric::distance::Euclidian<double> euclidianL2Distance;
auto result_1 = euclidianL2Distance(v0, v1);
std::cout << "result: " << result_1 << std::endl;
// out:
// Euclidian (L2) Metric
// result: 2
```

**Euclidian Threshold Metric**
``` cpp
metric::distance::Euclidian_thresholded<double> euclidianThresholdDistance(1000.0, 3000.0);
auto result_2 = euclidianThresholdDistance(v0, v1);
std::cout << "result: " << result_2 << std::endl;
// out:
// Euclidian Threshold
// result: 1000
```

**Manhatten/Cityblock (L1) Metric**
``` cpp
metric::distance::Manhatten<double> manhattenL1Distance;
auto result_3 = manhattenL1Distance(v0, v1);
std::cout << "result: " << result_3 << std::endl;
// out:
// Manhatten/Cityblock (L1) Metric
// result: 4
```

**Minkowski (L general) Metric**
``` cpp
metric::distance::P_norm<double> pNormDistance(2);
auto result_4 = pNormDistance(v0, v1);
std::cout << "result: " << result_4 << std::endl;
// out:
// Minkowski (L general) Metric
// result: 2
```

**Cosine Metric**
``` cpp
metric::distance::Cosine<double> cosineDistance;
auto result_5 = cosineDistance(v0, v1);
std::cout << "result: " << result_5 << std::endl;
// out:
// Cosine Metric
// result: 0.970143
```

### Euclidean Distance metric

### Edit Distance metric (for strings)

### Time Warp Elastic Distance metric (for curves)

### Structural Similartiy metric (for images)