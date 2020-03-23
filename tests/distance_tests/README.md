
---

## Run
*You need STL and C++17 support to compile.*

#### Using CMake

_Windows_

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