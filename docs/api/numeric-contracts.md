# `mtrc::numeric` Promoted Contracts

`mtrc::numeric` is METRIC's native, domain-neutral numerical core. It provides
small, stable numeric primitives used by the rest of the framework; it is **not**
a user-facing finite-metric-space API and owns no `record`, `space`, `metric`,
`stats`, `modify`, or `solve` concepts.

The core is **header-only and dependency-free by default**: `#include
<metric/numeric.hpp>` pulls only standard-library and `metric/numeric/...`
headers — no BLAS, LAPACK, Python, or NumPy. BLAS acceleration is off by default
(`METRIC_NUMERIC_BLAS_MODE=0`); LAPACK-backed routines are optional execution
paths that only require a link when actually called (see
[Optional backends](#optional-backends)).

## Curated facade headers

Each Level-2 area has a small curated facade header so callers can include exactly
the contract surface they need instead of the whole umbrella. Every facade
compiles standalone and re-exports promoted core headers only (it declares no
symbols of its own).

| Facade header | Area | Promoted contract surface |
| --- | --- | --- |
| `<metric/numeric/scalar.hpp>` | scalar | element shims (`abs`, `sqrt`, `pow`, `exp`, `log`, `sign`), `equal`, the `complex`/`real`/`imag` trio, `isDefault`, and the `accuracy`/`epsilon`/`inf` constants |
| `<metric/numeric/vector.hpp>` | vector | dense vectors (`DynamicVector`, `StaticVector`, `HybridVector`, `CustomVector`, `UniformVector`, `ZeroVector`, `InitializerVector`) and the `rowVector`/`columnVector`/`aligned`/`unaligned`/`padded`/`unpadded` construction tags |
| `<metric/numeric/matrix.hpp>` | matrix | dense matrices and the triangular/symmetric/Hermitian/diagonal adaptors, `rowMajor`/`columnMajor` and alignment/padding tags, the row/column/submatrix/subvector/diagonal/selection views, reductions/norms/elementwise maps (`sum`, `min`, `max`, `mean`, `prod`, `var`, `stddev`, `trans`, `ctrans`, `map`, `evaluate`, `norm`, `sqrNorm`), and the METRIC-authored `MatrixPrimitives.h` helpers |
| `<metric/numeric/sparse.hpp>` | sparse | `CompressedMatrix`, `CompressedVector` and their `append`/`finalize`/`insert`/`erase`/`reserve`/`nonZeros` mutation surface |
| `<metric/numeric/linear_algebra.hpp>` | linear_algebra | `inv`, `det`, `invert`, `invert2x2`, `llh`, `solve`, `eigen`, `svd` and the `asGeneral`/`asSymmetric` inversion flags |
| `<metric/numeric/graph.hpp>` | graph | raw graph primitives over integer indices and edge weights (`graph_shortest_path_distances`, symmetrization, degree/connectivity/stretch accumulators) |
| `<metric/numeric/random.hpp>` | random | `getSeed`, `setSeed`, `rand<T>`, `randomize`, and the `Rand<T>` specializations |
| `<metric/numeric/parallel.hpp>` | parallel | the shared-memory (SMP) execution surface, single-threaded and off by default |
| `<metric/numeric/io.hpp>` | io | the self-contained `Archive` plus the dense matrix/vector serializers |

`<metric/numeric.hpp>` remains the full umbrella; the facades are narrower entry
points for library boundaries.

## Stability of each area

| Area | Stability | Notes |
| --- | --- | --- |
| scalar | stable | shims/`complex`/`isDefault` are stable and tested; `accuracy`/`epsilon`/`inf`/`equal` are promoted constants/comparison now covered by the facade smoke; `sign` is thin (one external caller) |
| vector | stable | container family is load-bearing (`DynamicVector` dominant); `CustomVector` is a non-owning view (caller owns the buffer lifetime); `UniformVector` is thin |
| matrix | stable | the largest external surface; containers, adaptors, views, reductions and `MatrixPrimitives` are all externally used and smoke-covered |
| sparse | stable | two container classes plus mutation; the `insert` duplicate-key contract throws `std::invalid_argument` |
| linear_algebra | stable (LAPACK-optional) | `invert2x2` and small kernels are link-free; `inv`/`det` (large), `llh`, `solve`, `eigen`, `svd` require a LAPACK link at call time |
| graph | stable | a single coherent header; the principal consumer is `metric/space/index/operators.hpp` |
| random | stable | seeded reproducibility within one process; RNG state is process-global and not thread-safe |
| parallel | internal / thin | the default build is single-threaded; the generic `Thread`/`ThreadPool` utilities have no METRIC caller and are removal candidates, intentionally not promoted by the facade |
| io | stable | `Archive` is the promoted contract; the per-type serializers are internal machinery |

## Optional backends

- **BLAS** is off by default through `METRIC_NUMERIC_BLAS_MODE=0`. Every
  `metric/numeric/math/blas/**` kernel is `#if`-compiled out under the default,
  so including the headers introduces no BLAS dependency. Enabling BLAS is an
  explicit build choice and may require a compatible CBLAS header and library.
- **LAPACK**-backed solver and decomposition routines (`eigen`, `svd`, `llh`,
  `solve`, and `inv`/`det` for larger matrices) are optional execution paths.
  Header inclusion is link-free; calling those routines requires linking an
  appropriate LAPACK implementation. The explicit small kernels (`invert2x2`,
  small determinants) are pure header-only.
- The external-SIMD switches (`METRIC_NUMERIC_USE_SLEEF`,
  `METRIC_NUMERIC_USE_XSIMD`) are off by default; their headers are never
  included unless explicitly enabled.

No public C++ consumer is forced into CMake to compile header-only numeric code.
