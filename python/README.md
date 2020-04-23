# METRIC-PY
A python wrapper for METRIC library (https://panda.technology/de/metric)
# Installation
You need Python 3.6+
## Linux & OS X
```
python -m pip install metric-py -i https://test.pypi.org/simple/
```
## Windows (x64 only)
You will need to install any BLAS implementation. 
The easiest way is by using [Miniconda](https://docs.conda.io/en/latest/miniconda.html):
```bash
conda config --add channels conda-forge
conda update -n base conda -y
conda install -c conda-forge libopenblas openblas -y
```
Then you can use pip to install
```
python -m pip install metric-py -i https://test.pypi.org/simple/
```
# Build from the source
```
git clone --recurse-submodules https://github.com/panda-official/metric
```
Download and extract [Boost](https://www.boost.org/users/download/) (1.67+).
For Windows there are pre-build binaries available.
## Install Prerequisites
### Ubuntu
```
sudo apt-get install cmake
sudo apt-get install libboost-all-dev
sudo apt-get install libopenblas-dev
```
### Windows
Install [Miniconda](https://docs.conda.io/en/latest/miniconda.html).
In Conda CLI initialize your virtual environment with desired Python version:
```bash
conda create --name my_env -y python=3.8
conda activate my_env
```
Install OpenBLAS from alternative repo
```bash
conda config --add channels conda-forge
conda update -n base conda -y
conda install -c conda-forge libopenblas openblas -y
```
## Build package
At least 2GB of RAM is required
```
python setup.py bdist_wheel
```
to limit memory usage during build add `MAKE="make -j1"`:

```
MAKE="make -j1" python3 setup.py bdist_wheel
```
### Install module
```
python -m pip install dist/*
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
