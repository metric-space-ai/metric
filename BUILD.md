
The following dependencies are required for building examples and tests:

1. C++ compiler supported C++17
2. Cmake 3.10+ (https://www.cmake.org)
3. Boost libraries (https://www.boost.org)
4. BLAS and LAPACK libraries
5. Libpqxx (for SensorExample)

# Ubuntu
### Install dependencies
On Ubuntu one can install dependencies from Ubuntu repositories.

Note that CMake 3.10+ is available in default repositories since Ubuntu 18.04 LTS.
If you are using an earlier version of Ubuntu, you may need to build CMake from [source code](https://gitlab.kitware.com/cmake/cmake).

```
$ sudo apt install cmake
$ sudo apt install libboost-all-dev
$ sudo apt install libopenblas-dev
$ sudo apt install libpqxx-dev postgresql-server-dev-all
```
### Build

If you are building it from inside a virtual machine, make sure to provision at least **2 GiB of virtual memory** for the build process to succeed.

```
  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release ../
  $ make
```

# MacOS X

### Install dependencies
Use Homebrew tool to install dependencies

```
$ brew install cmake
$ brew install boost
$ brew install openblas
$ brew install libpqxx
```

### Build
```
  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release ../
  $ make
```

# Windows

### Install dependencies
On Windows one can install some dependencies using vcpkg (https://github.com/microsoft/vcpkg)

```
> vcpkg install --triplet=x64-windows-static libpqxx
> vcpkg install --triplet=x64-windows-static boost
```
Install Intel MKL Library (https://software.intel.com/en-us/mkl/choose-download/windows)

### Build
```
> mkdir build
> cd build
> cmake -DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg_root]\scripts\buildsystems\vcpkg.cmake ../
> cmake --build .
```

