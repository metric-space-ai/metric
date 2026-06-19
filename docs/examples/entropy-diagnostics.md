# Entropy Diagnostics

Metric-space entropy is an expert diagnostic on a finite metric space. It estimates local freedom in the dataset from distances, so the metric remains part of the diagnostic instead of being hidden behind a vector embedding.

The same operator works across record families when each family has an explicit metric:

- numeric records with Chebyshev distance
- string records with edit distance
- structured records with a domain metric

The promoted C++ example [metric_space_entropy.cpp](../../examples/core/metric_space_entropy.cpp) runs in the core CI path and asserts regression values for numeric and string records.

## Current C++ Shape

```cpp
#include <metric/correlation/entropy.hpp>
#include <metric/distance.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {
    "AAA",
    "HJGJHFG",
    "BBB",
    "AAAA",
    "long long long long long long string",
    "abcdefghjklmnopqrstuvwxyz",
};

metric::Entropy<void, metric::Edit<int>> entropy(metric::Edit<int>(), 3, 2);
double value = entropy(records);
```

The restored Python core wheel does not promote entropy bindings yet. Entropy belongs in the Expert API because interpretation depends on metric assumptions, neighborhood size, sampling density, and scale.
