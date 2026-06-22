# Revival Status

The revival foundation is closed when the repository satisfies the current
hub plan in `.codex/plans/REVIVAL_PLAN.md`.

Current status:

- public plans are consolidated under `.codex/plans`
- root-level plan-file pollution has been removed
- the active C++ source layout has Level-1 homes for `record`, `space`,
  `metric`, `stats`, `modify`, `solve`, and `numeric`
- Level-2 homes are physically present for the promoted C++ framework areas:
  record conversion/validation, space storage/index/select/split/merge,
  metric catalog/admission/discovery, stats search/properties/correlate/
  structural analysis, modify represent/reduce/expand/resample/map/dynamics/
  compose, solve parametric/laplacian, and numeric primitives
- README, concept docs, C++ API docs, and the static site explain finite metric
  spaces before naming concrete algorithms
- metric discovery and quarantine docs separate admitted metrics from unproven
  or non-metric pairwise functions
- real algorithms and metric math remain native C++; Python is an adapter layer
- current C++ integration gate is green: `142/142` CTest tests under
  `build/dependency-core`
- current Python adapter gate is green: `38` tests plus `13` subtests, including
  the native mapping-pipeline adapter

The revival cleanup phase is now an integration checkpoint. Next phase work is
feature completion and optimization: metric admission, missing literature
metrics, production-grade hero applications, performance, and API hardening.
