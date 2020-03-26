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
pip3 install dist/*
```
## Examples

```python
import numpy
from metric.distance import entropy

aent = numpy.float_([
    [5.0, 5.0],
    [2.0, 2.0],
    [3.0, 3.0],
    [5.0, 1.0],
])

print("Entropies:")
for metric in ('chebyshev', 'p-norm', 'euclidean', 'manhatten'):
    res = entropy(aent, 3, 2.0, metric=metric)
    print(f'using {metric}: {res:.5f}')

res = entropy(aent)
print(f'using Default: {res:.5f}')
```
for more examples please check `examples/` folder