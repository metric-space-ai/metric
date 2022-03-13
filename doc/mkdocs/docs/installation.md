# Installation

## Cmake install

1. Download and unpack [last release](https://github.com/panda-official/metric/releases/latest)
1. Run cmake install (replace `{latest}` for your version)

```shell
cd metric-{latest}
mkdir build && cd build
cmake .. && make install
```

# Integration

## Using cmake target

```cmake
find_package(panda_metric REQUIRED)

add_executable(program program.cpp)
target_link_libraries(program panda_metric::panda_metric)

# metric use blaze as linear algebra library which expects you to have a LAPACK library installed
# (it will still work without LAPACK and will not be reduced in functionality, but performance may be limited)
find_package(LAPACK REQUIRED)
target_link_libraries(program ${LAPACK_LIBRARIES})
```

## Include headers

Main header for whole metric library:

```cpp
#include <metric/metric.hpp>
```

You also could use module specific header:

```cpp
#include <metric/mapping.hpp>
```

## Python bindings

You can install it as Python lib with `pip`.

```
python -m pip install metric-py -i https://test.pypi.org/simple/
```

Check out the Python subdirectory for more helps and system requirements.
