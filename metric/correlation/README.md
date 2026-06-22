# METRIC | CORRELATION

*A templated, header only C++17 library for nonlinear correlation of arbitrary data structures and types.*

## Overview

This module measures the **dependence** between two paired finite metric spaces: given the same
observations described under two (possibly different) metrics, it answers "are these two spaces
dependent, and how strongly?". The result is a *dependence/correlation statistic*, **not a metric
or distance** — it does not measure how far apart two records are, and it is never admitted as a
metric.

Use MGC (Multiscale Graph Correlation) as that statistic to find nonlinear dependencies. It is
optimized for small data set sizes. The sample statistic lies in `[-1, 1]` (≈1 strong dependence,
≈0 none); no p-value or significance test is produced here — significance testing is a separate
concern. The two inputs must be paired and have the same record count.

*Multiscale Graph Correlation, Bridgeford, Eric W and Shen, Censheng and Wang, Shangsi and Vogelstein, Joshua, 2018, doi
= 10.5281/ZENODO.1246967*

## Simple use

Compute the mgc correlation based on two datasets A and B.

If you don't have distance matrices, yo can build them at first (euclidean metric will be used with this embedded
functions).

```C++
auto A = mtrc::distance_matrix(dataset); // std::vector<std::vector<double>> dataset
```

Then just create MGC object and call `()` operator:

```C++
typedef std::vector<int> Record;
typedef mtrc::Euclidean<double> Distance;

auto mgc_corr = mtrc::MGC<Record, Distance, Record, Distance>();
auto result = mgc_corr(A, B);

// out:
// result: 0.626877
```

*For a full example and more details see `examples/correlation_examples/simple_example.cpp`*

## Estimate

For large datasets computing of the correlation can take a lot of time, then MGC has the `estimate()` method, which will
save a lot of time. For use it just call `estimate()` on MGC object:

```C++
typedef std::vector<int> Record;
typedef mtrc::Euclidean<double> Distance;

auto mgc_corr = mtrc::MGC<Record, Distance, Record, Distance>();
auto result = mgc_corr.estimate(A, B);

// out:
// result: 0.626877
```

*For a full example and more details see `examples/correlation_examples/advanced_example.cpp`*

But the real power with mgc is to compare different types and different metrics. Therefor use mgc as functor.

## Function (object) / functor with user types and metrics

At first here are some types and metrics:

```C++

struct simple_user_euclidian {

    double operator()(const std::vector<double>& a, const std::vector<double>& b) const
    {
        double sum = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }
};

typedef std::vector<double> Rec1;
typedef std::array<float,3> Rec2;
typedef simple_user_euclidian Met1;
typedef mtrc::Manhattan<float> Met2;
````

Now build the function (object) and compute the mgc again:

```C++
auto mgc_corr = mtrc::MGC<Rec1, Met1, Rec2, Met2>();
auto result2 = mgc_corr(dataset1, dataset2);
```

dataset1 and dataset2 could be different containers, but must correspond to the same data record IDs of course, so they
must have same sizes.

*For a full example and more details see `examples/correlation_examples/advanced_example.cpp`*


---

## Run

*You need STL and C++14 support to compile.*

METRIC | CORRELATION works headonly. Just include the header into your project.

```cpp
#include "metric/correlation.hpp"
```

or directly include one of specified distance from the following:

```cpp
#include "metric/correlation/mgc.hpp"
```

#### CMake compilation

Inside folder with your cpp file or inside `examples/correlation_examples/` run the following commands:

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
$ clang++ ./examples/correlation_examples/simple_example.cpp -std=c++17
```
