# Histogram Space With EMD

Histograms and empirical distributions are often better compared by transport cost than by coordinate-wise vector distance. Earth mover distance models how much mass must move between bins and how expensive those bin-to-bin moves are.

The promoted C++ example [histogram_emd_space.cpp](../../examples/core/histogram_emd_space.cpp) uses a small one-dimensional ground-cost matrix, then builds a `metric::MatrixSpace` over histogram records.

Core shape:

```cpp
#include <metric/distance.hpp>
#include <metric/space.hpp>

#include <vector>

using Histogram = std::vector<double>;

std::vector<std::vector<double>> ground_cost = {
    {0.0, 1.0, 2.0, 3.0},
    {1.0, 0.0, 1.0, 2.0},
    {2.0, 1.0, 0.0, 1.0},
    {3.0, 2.0, 1.0, 0.0},
};

metric::EMD<double> emd(ground_cost, 3.0);

std::vector<Histogram> records = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
};

metric::MatrixSpace<Histogram, metric::EMD<double>> space(records, emd);
```

This is still a vector container at the storage level, but the geometry is not a norm on coordinates. The metric defines movement of mass across bins before METRIC builds the finite metric space.
