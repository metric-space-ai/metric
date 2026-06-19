# Examples

The examples tree contains both the revived public examples and broader historical examples.

## Core Revival Examples

`examples/core/` is the promoted example set for the revival. These examples are small, deterministic, non-vector-first where possible, and executed by the `core` and `dev` CTest presets.

The current core examples cover:

- string/edit metric spaces
- custom metric callables
- explicit matrix, graph, and tree representations
- entropy diagnostics
- intrinsic-dimension diagnostics
- MGC correlation between metric spaces
- TWED for time-series records
- EMD for histogram records

These examples are release-gate examples. A failure here is a core regression.

## Historical and Extended Examples

The other example folders preserve broader project history:

- `correlation_examples/`
- `distance_examples/`
- `dnn_examples/`
- `energies_examples/`
- `ensemble_examples/`
- `laplacians_example/`
- `mapping_examples/`
- `space_examples/`
- `transform_examples/`
- `utils_examples/`

Those folders are not first-page API promises and are not revival release gates. They should be promoted one area at a time only after they have deterministic expected output, clear docs, and CI coverage.

The support status for each area is tracked in [../docs/stability.md](../docs/stability.md). The CI boundary is documented in [../docs/testing-and-ci.md](../docs/testing-and-ci.md).
