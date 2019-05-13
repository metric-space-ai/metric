# metric_distance
A templated, header only C++14 implementation of a Metric Distance Functions. 

## Standart metrics

Suppose we have a table with two records:
```cpp
std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
```

Using **METRIC** framework we can calculate a set of standart metrics for this records. 

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

## Euclidean Distance metric

## Edit Distance metric (for strings)

## Time Warp Elastic Distance metric (for curves)

## Structural Similartiy metric (for images)