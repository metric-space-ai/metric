# METRIC | CORRELATION
*A templated, header only C++14 library for nonlinear correlation of arbitrary data structures and types.*

## Overview

Uses MGC (Multiscale Graph Correlation)
as correlation coefficient to find nonlinear dependencies in data sets. It is optimized for small data set sizes.

*Multiscale Graph Correlation, Bridgeford, Eric W and Shen, Censheng and Wang, Shangsi and Vogelstein, Joshua, 
2018, doi = 10.5281/ZENODO.1246967*

## Simple use 

Compute the mgc correlation based on two datasets A and B.

If you don't have distance matrices, yo can build them at first (euclidian metric will be used with this embedded functions).

```C++
auto A = metric::distance_matrix(dataset) // std::vector<std::vector<double> dataset 
```

Then just create MGC object and call `()` operator:

```C++
typedef std::vector<int> Record;
typedef metric::Euclidian<double> Distance;

auto mgc_corr = metric::MGC<Record, Distance, Record, Distance>();
auto result = mgc_corr(A, B);

// out:
// result: 0.626877
```

*For a full example and more details see `examples/correlation_examples/simple_example.cpp`*



## Estimate

For large datasets computing of the correlation can take a lot of time, then MGC has the `estimate()` method, 
which will save a lot of time. For use it just call `estimate()` on MGC object:

```C++
typedef std::vector<int> Record;
typedef metric::Euclidian<double> Distance;

auto mgc_corr = metric::MGC<Record, Distance, Record, Distance>();
auto result = mgc_corr.estimate(A, B);

// out:
// result: 0.626877
```

*For a full example and more details see `examples/correlation_examples/advanced_example.cpp`*

But the real power with mgc is to compare different types and different metrics. Therefor use mgc as functor.

##  Function (object) / functor with user types and metrics


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
typedef metric::Manhatten<float> Met2;
````
Now build the function (object) and compute the mgc again:
```C++
auto mgc_corr = metric::MGC<Rec1, Met1, Rec2, Met2>();
auto result2 = mgc_corr(dataset1, dataset2);
```
dataset1 and dataset2 could be different containers, but must correspond to the same data record IDs of course, so they must have same sizes.

*For a full example and more details see `examples/correlation_examples/advanced_example.cpp`*


---

## Run
*You need STL and C++14 support to compile.*

METRIC | correlation works headonly. Just include the header into your project.

```cpp
#include "modules/correlation.hpp"
```

or directly include one of specified distance from the following:

```cpp
#include "modules/correlation/mgc.hpp"
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
$ clang++ ./examples/correlation_examples/simple_example.cpp -std=c++14
```
