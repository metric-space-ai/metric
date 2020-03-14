# Ubuntu
## Install dependencies
On Ubuntu one can install dependencies from Ubuntu repositories.

```
$ sudo apt-get install python-numpy
```

## Build module

```
python3 setup.py bdist_wheel
```

to limit memory usage for build add `MAKE="make -j1"`:

```
MAKE="make -j1" python3 setup.py bdist_wheel
```

## Install module

sudo pip3 install dist/metric-&lt;version&gt;-cp37-cp37m-linux_&lt;arch&gt;.whl

## [Examples](examples)
