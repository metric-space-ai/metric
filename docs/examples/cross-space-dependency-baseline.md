# Cross-Space Dependency Baseline

This report records the first CI-safe cross-space dependency hero fixture for
METRIC production readiness.

Command:

```bash
build/core/examples/engine/engine_cross_space_mgc
```

Fixture:

- left records are process scalar observations with Euclidean distance
- right records are paired quality observations with Manhattan distance
- METRIC path compares the two induced finite metric spaces with MGC
- baseline path treats the two modalities as raw one-dimensional vectors and
  performs nearest-neighbor pairing across modalities without calibrating their
  units or nonlinear relationship

Expected output:

```text
cross-space MGC = 1
cross-space paired records = 12
cross-space raw-vector pairing correct = 3/12
cross-space raw-vector mismatches = 9/12
```

Interpretation:

- MGC detects the dependency between the two paired metric spaces
- raw cross-modal vector nearest-neighbor pairing misses most paired records
  because the quality coordinate is a nonlinear recoding of the process
  coordinate
- the demo keeps the two record containers and metrics separate; the shared
  observation order is the alignment contract
