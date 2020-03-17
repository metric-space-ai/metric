
---

## Run
*You need STL and C++17 support to compile.*

#### Using CMake

_Windows_

- First of all you need to install [Intel MKL](https://software.intel.com/en-us/mkl)
- Then copy `mkl` (usually from `C:\Program Files (x86)\IntelSWTools\compilers_and_libraries\windows`) folder to the root of the project. 
- Then you can run cmake to create project (links to `mkl` already inside `CMakeLists.txt`)
```bash
mkdir build
cd build
cmake .. -A x64 -DBoost_NAMESPACE="libboost" -DBoost_COMPILER="-vc141" -DBOOST_ROOT=C:/boost_1_70_0/
```

Where `DBoost_NAMESPACE` and `DBoost_COMPILER` params point to suffixes and postfixes of the search libs in the `stage/lib` folder of the boost root directory. 
And `DBOOST_ROOT` point to the boost root folder. 

Then open solution in the Microsoft Visual Studio.

_Linux_

Just run cmake
```bash
mkdir build
cd build
cmake ..
make
```