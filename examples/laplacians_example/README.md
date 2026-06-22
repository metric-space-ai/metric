# Graph-Laplacian solver example

This example solves linear systems in a finite metric space's graph Laplacian —
the electrical-flow / effective-resistance view of a metric space whose records
are graph nodes. It exercises `mtrc::solve::laplacian` (the combinatorial
Laplacian, the approximate-Cholesky preconditioner, and PCG) on grid and
path-graph fixtures.

## Build requirements

- STL and C++17 support.
- The example links only the header-only `metric` target (see `CMakeLists.txt`);
  it adds no external library of its own.
- LAPACK is an *optional* backend. The dense (<50-node) Cholesky path uses LAPACK
  (`dpotrf`/`dpotrs`) when available; it is not a mandatory dependency of the
  public core. See `metric/numeric/README.md` for the optional-backend policy.

## Run

### Using CMake

_Linux / macOS_

```bash
mkdir build
cd build
cmake ..
make
```

_Windows_

```bash
mkdir build
cd build
cmake .. -A x64
```

Then open the generated solution in Microsoft Visual Studio.
