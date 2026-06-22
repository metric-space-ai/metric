# METRIC Numeric Core

`mtrc::numeric` is METRIC's native numerical core.

The code in this tree is maintained as first-party METRIC C++: public names
live under `mtrc::numeric`, headers are included from `metric/numeric/...`, and
callers depend on METRIC numeric contracts.
The core is header-only for its default dense, sparse, graph, random, and small
linear-algebra paths.

Historical source provenance is retained for legal traceability. The
BSD-3-Clause notice in `LICENSE.BSD-3-Clause` documents the inherited license
obligations for the portions that require it; that provenance does not define
the architecture, API, package identity, or maintenance model of
`mtrc::numeric`.

Optional backend boundaries:

- BLAS acceleration is off by default through `METRIC_NUMERIC_BLAS_MODE=0`.
  Enabling it is an explicit build choice and may require a compatible CBLAS
  header and library.
- LAPACK-backed solver and decomposition routines are optional execution paths.
  Header inclusion does not make LAPACK a mandatory dependency, but calling
  those routines requires linking an appropriate LAPACK implementation.

Rules for work in this tree:

- keep public docs and comments centered on METRIC-owned numeric contracts
- do not reintroduce external includes, namespaces, build dependencies, or
  package lookup as default requirements
- preserve historical license notices where required
- make public APIs read as METRIC-owned numeric contracts
- remove generic machinery when a METRIC contract does not need it
- raise adjacent METRIC code to the same C++ quality level where this core is
  stronger
