# METRIC | TRANSFORM
*A templated, header only C++17 implementation of a Metric Transform Functions.*

## Overview

## Examples

Suppose we have a linespace data:
```cpp
auto x = wavelet::linspace(0.0, 1.0, 3200);
```

Using **METRIC** framework we can reduce the data. 

For this, we can write the following function:
``` cpp
template <typename T>
std::vector<T>
extract_energies(std::vector<T> x)
{
    auto [x0, tmp1] = wavelet::dwt(x, 5);
    auto [x1, tmp2] = wavelet::dwt(x0, 5);
    auto [x11, tmp3] = wavelet::dwt(x1, 5);
    auto [x111, x112] = wavelet::dwt(x11, 5);
    auto [x1111, x1112] = wavelet::dwt(x111, 5);
    auto [x1121, x1122] = wavelet::dwt(x112, 5);
    auto [x11111, x11112] = wavelet::dwt(x1111, 5);
    auto [x11121, x11122] = wavelet::dwt(x1112, 5);
    auto [x11211, x11212] = wavelet::dwt(x1121, 5);
    auto [x11221, x11222] = wavelet::dwt(x1122, 5);
    std::vector<std::vector<T>> subbands = {x11111, x11112, x11121, x11122, x11211, x11212, x11221, x11222};

    std::vector<T> energies(subbands.size());
    for (int i = 0; i < subbands.size(); ++i)
    {
        T sum = 0;
        for (int j = 0; j < subbands[i].size(); ++j)
        {
            sum += subbands[i][j] * subbands[i][j];
        }
        energies[i] = sum / T(subbands[0].size());
    }
    std::cout << subbands[0].size() << std::endl;
    return energies;
}
``` 

and then use above function for our data:
``` cpp
auto result = extract_energies(x);
// out:
// result: [19.288, 4.983e-06, 1.56103e-07, 1.60135e-06, 4.63031e-07, 8.45242e-08, 2.23311e-07, 3.72132e-08]
```

*For a full example and more details see `examples/transform_examples/dwt_reducer.cpp`*

---

## Run
*You need STL and C++17 support to compile.*

METRIC | TRANSFORM works headonly. Just include the header into your project.

```cpp
#include "modules/transform.hpp"
```

or directly include one of specified distance from the following:

```cpp
#include "transform/discrete_cosine.hpp"
#include "transform/wavelet.hpp"
```

#### Using CMake

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
$ clang++ ./examples/transform_examples/dwt_reducer.cpp -std=c++17
```