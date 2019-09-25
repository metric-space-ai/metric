
---

## Run
*You need STL and C++14 support to compile.*

METRIC | space works headonly. Just include the header into your project.

#### Using CMake

_Windows_

- First of all you need to install [Intel MKL](https://software.intel.com/en-us/mkl)
- Then copy `mkl` (usually from `C:\Program Files (x86)\IntelSWTools\compilers_and_libraries\windows`) folder to the root of the project. 
- Then you can run cmake to create project (links to `mkl` already inside `CMakeLists.txt`)
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