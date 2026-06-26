# Solver Trace Native Evidence Report

Date: 2026-06-26

## Evidence Source

`examples/engine/solver_trace_visual_export.cpp` exports
`docs/examples/assets/solver-trace/metric.visual.json` as native
`metric.visual.v1` evidence.

The exporter builds a deterministic finite metric fixture, derives a dense
Euclidean metric relation, constructs a metric-affinity graph, forms the
positive-definite smoothing system `(I + lambda L) x = b`, and runs native C++
Jacobi iterations. JavaScript does not compute residuals, objectives,
convergence values or solver iterations.

The exported provenance includes:

- `provenance.generator: "native-cpp"`
- `provenance.computed_by: "native C++ metric-Laplacian Jacobi solver"`
- `provenance.synthetic_js: false`
- `provenance.native_export: true`
- `provenance.native_checks_pass: true`

## Exported Trace Fields

The document contains one solver timeline, `native-laplacian-jacobi`, with 17
native iteration steps. Each step includes:

- `iteration`
- `residual`
- `objective`
- `step_delta`
- `solution_norm`

The same values are exposed as timeline-step scalar properties:

- `jacobi-residual`
- `jacobi-objective`
- `jacobi-step-delta`
- `jacobi-solution-norm`

Record-level preview context is available through the input signal and final
solution properties:

- `observed-signal`
- `final-solver-solution`

## Command Behavior

`visual/tools/check-visual-command-api.mjs` now loads
`docs/examples/assets/solver-trace/metric.visual.json` for `showSolverTrace()`.
The command must report:

- selected command `showSolverTrace`
- selected view kind `solver-trace`
- evidence kind `native`
- a `CurveRibbonLayer` descriptor from exported timeline-step evidence

`SolverTraceView` still only adapts exported series into render descriptors. It
now carries native provenance metadata on the solver-trace descriptor when the
input document has `provenance.native_export === true`.

## Validation

Passed:

- `cmake --preset core`
- `cmake --build build/core --target engine_solver_trace_visual_export -j 4`
- `./build/core/examples/engine/engine_solver_trace_visual_export --export-dir docs/examples/assets/solver-trace`
- `node visual/tools/check-visual-document.mjs docs/examples/assets/solver-trace/metric.visual.json`
- `node visual/tools/check-visual-command-api.mjs`
- `node visual/tools/check-views.mjs`
- `node visual/tools/check-public-gallery-evidence.mjs`
- `node visual/tools/check-grae10-golden.mjs`
- `ctest --test-dir build/core -R 'example_engine_solver_trace_visual_(export|validate)' --output-on-failure`

## Remaining Blockers

No blocker remains for backing `showSolverTrace()` with native METRIC visual
evidence. This is not a public hero promotion; the exported asset remains
`public_hero_ready: false`.
