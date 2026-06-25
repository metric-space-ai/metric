# Revival Status

This page records the public revival checkpoint of METRIC: the repository has a
coherent finite-metric-space vocabulary, native C++ ownership for the framework
layers, and reproducible examples that show the intended user workflows.

Current status:

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
- copyable C++ application templates cover time series, distributions, mixed
  records, cross-space dependence, record-set reduction, and derived coordinate
  spaces
- real algorithms and metric math remain native C++; Python is an adapter layer
- the core C++ and adapter gates are expected to stay green before changes are
  merged to `main`

The revival cleanup phase is now an integration checkpoint. Next phase work is
feature completion and optimization.

Production-readiness tracks:

- metric catalog completion and per-metric discovery documentation
- space/search/storage and stats hardening
- modify/map/dynamics hardening, including parametric diffusion coordinate as a composed finite-space
  mapping workflow
- solve/numeric quality for Laplacian, PCG, native DNN, and promoted numeric
  contracts
- binding and package gates that keep Python adapter-only
- hero applications with reproducible C++ evidence and public documentation

Active hero work starts with the licensed UCR process-curve condition-monitoring
package described in [Hero Application Visualization Plan](hero-application-visualization-plan.md).
