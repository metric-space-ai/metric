# METRIC-PY
A python wrapper for METRIC library (https://panda.technology/de/metric)
# Installation
## Ubuntu
```
pip3 install metric-py -i https://test.pypi.org/simple/
```

# Build from the source
## Ubuntu
```
git clone https://github.com/panda-official/metric
```
for general requirement please follow the instructions in the BUILD.md
### Install requirements
```
sudo apt-get install python-numpy
```
### Build package
```
python3 setup.py bdist_wheel
```

to limit memory usage during build add `MAKE="make -j1"`:

```
MAKE="make -j1" python3 setup.py bdist_wheel
```

### Install module
## Linux
```
python3 -m pip install dist/*
```
## Windows
TODO
## OS X
```
python3 -m pip install dist/*
```
## Examples

```python
import numpy
from metric.correlation import Entropy
from metric.distance import Euclidean, P_norm, Manhatten

aent = numpy.float_([
    [5.0, 5.0],
    [2.0, 2.0],
    [3.0, 3.0],
    [5.0, 1.0],
])

print("Entropies:")
for metric in (Euclidean, P_norm, Manhatten):
    res = Entropy(metric=metric(), p=3, k=2)(aent)
    print(f'using {metric}: {res:.5f}')

res = Entropy(p=3, k=2)(aent)
print(f'using Default: {res:.5f}')
```
for more examples please check `examples/` folder

## For developers
One of the core idea of metric-py is to find the balance between performance of C++ and the beauty of Python.
By its nature METRIC library heavily relies on compile time generation with multiple level of C++ templates.
Even tiny C++ code snippet could generate tens of classes, on the bright side you pay only for what you use.
Python in its turns lacks compile time optimization, so to stay on level it needs to have multiple variants of the same
code, but generated with different parameters and scenarios of use.
In general Metric-py uses two approaches to decide which implementation to use:
- class factories
- dynamic routing

### Class Factories
Class factories in general a functions that mimics class constructor. They use C++ function overloading mechanism, the
concrete function overload will return only specific implementation. It means that class implementation should be completely
described by function arguments. One example of class factories is Entropy factory:
```
template <typename Metric = metric::Euclidean<double>()>
metric::Entropy<void, Metric> createEntropy(
    const Metric& metric,
    size_t k = 7,
    size_t p = 70,
    bool exp = false
){
    return metric::Entropy<void, Metric>(metric, k, p, exp);
}
```

### Dynamic routing
This approach construct the name of implementation class/function from arguments of factory: