# Tests

The tests tree separates the revived core gate from broader historical coverage.

## Core Revival Tests

`tests/core_smoke/` is the fast C++ release gate. It is built by the `core` and `dev` presets and covers:

- basic finite-space construction
- metric contract checks
- matrix, graph, and tree consistency
- MGC regression values
- entropy regression values
- promoted core examples through CTest

`tests/downstream_consumer/` verifies the installed CMake package with `find_package(panda_metric)`. It is part of the C++ core CI gate.

The Python core CI gate under `python/tests/core/` covers the revived Python facade, promoted Python examples, custom metric callables, and metric contract checks for built-in, NumPy, and structured-record metrics.

## Historical and Extended Tests

The other test folders preserve broader historical coverage:

- `correlation_tests/`
- `distance_tests/`
- `dnn_tests/`
- `ensembles_tests/`
- `image_processing_tests/`
- `mapping_tests/`
- `space_tests/`
- `sparsification_tests/`
- `transform_test/`
- `utils/`
- `wrapper_tests/`

These folders are not revival release gates yet. They are restoration candidates. Promote one folder or capability at a time only after it has deterministic fixtures, explicit expected values or contract assertions, and CI coverage.

The release-gate boundary is documented in [../docs/testing-and-ci.md](../docs/testing-and-ci.md). Module status is documented in [../docs/stability.md](../docs/stability.md).
