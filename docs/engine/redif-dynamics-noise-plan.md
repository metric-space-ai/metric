# Redif Dynamics And Noise Production Plan

Status date: 2026-06-26
Status: completed production-readiness plan
Primary contract: `docs/concepts/redif-metric-dynamics.md`

This file is the editable TODO plan for metric-induced dynamics, Redif inverse
dynamics, and noise treatment in METRIC. It is written as an agent handoff file:
update checkbox state, owner, evidence, and notes as work lands.

## Status Legend

- `[ ]` not started
- `[~]` in progress, with evidence still missing or incomplete
- `[x]` done, with tests/docs/evidence linked
- `[!]` blocked by a design decision, external dependency, or failing gate

## Ground Rules

- Treat noise as dynamics over a finite metric space.
- Adding noise means forward metric-induced dynamics.
- Removing noise means inverse metric-induced dynamics.
- Public vocabulary must stay in finite metric-space language: atoms, measures,
  transition, affinity, local scale, entropy, transport path, singularity,
  representatives, density, quotient, derived space.
- Machine-learning terms are allowed only in explicit comparison sections or
  implementation-detail notes.
- Do not mark a section done only because code compiles. The mathematical claim
  must be documented and tested.

## Current Baseline

- Redif implementation: `metric/modify/dynamics/redif.hpp`
- Concept contract: `docs/concepts/redif-metric-dynamics.md`
- C++ API docs: `docs/api/cpp.md`
- Python API docs: `docs/api/python.md`
- Focused C++ Redif/visual tests currently pass:
  `ctest --test-dir build/core -R
  'redif_.*_smoke|redif_visual_export_validate|example_engine_redif_metric_dynamics_visual_export'
  --output-on-failure` passed 10/10 on 2026-06-26.
- Python Redif/Revival tests currently pass:
  `python -m unittest python.tests.core.test_redif_python_surface python.tests.core.test_revival_api`
  passed 65/65 on 2026-06-26.
- Full native CTest currently passes:
  `ctest --test-dir build/core --output-on-failure` passed 202/202 on
  2026-06-26; MNIST gallery/calibration examples were skipped by their existing
  skip guards.
- Full core rebuild currently passes:
  `cmake --build build/core -j2` completed on 2026-06-26 after the sparse,
  sampled/chunked, transport-budget, and screenshot work.

The current operator uses a symmetric self-tuned heat affinity:

```text
K_ij = exp(-ell_ij^2 / (sigma_i sigma_j))
```

where `ell_ij` is the local k-neighbour metric relation and `sigma_i` is the
positive local scale derived from nonzero local distances.

## Open TODO Dashboard

This is the production-readiness gate list that was closed for Redif dynamics
and noise treatment. Details are tracked in the milestone sections below.

- [x] Finish multiscale Redif path sensitivity documentation.
- [x] Add stronger canonical multiscale ranking tests for isolated atoms and
  bridge atoms across parameter grids.
- [x] Finish malformed distance-matrix failure tests, including non-finite and
  asymmetric cases.
- [x] Verify full relabelling invariance for Redif measure paths and path
  rankings, not only local operator affinities.
- [x] Audit the exact Wasserstein path computation and document remaining
  complexity limits.
- [x] Add spectral-gap or accepted proxy diagnostics to per-step operator
  diagnostics.
- [x] Add runtime-policy support for Redif dense budgets, future sparse-provider
  budgets, and refusal diagnostics.
- [x] Add finite metric-space examples for strings, histograms, process curves,
  mixed records, density-vs-Redif disagreement, and forward noise.
- [x] Add `metric.visual.v1` export for native Redif measure paths.
- [x] Add visual validation proving JavaScript consumes native evidence instead
  of recomputing Redif dynamics.
- [x] Define sparse Redif semantics and add dense/sparse parity tests.
- [x] Define chunked/sampled Redif semantics and diagnostics before
  implementation.
- [x] Add transport scaling budgets and optional exactness-preserving support
  truncation semantics.
- [x] Add measure-path and inverse-dynamics diagrams to concept docs.
- [x] Run full CTest, Python source smoke, docs vocabulary scan, and
  `git diff --check`; wheel smoke is recorded as not applicable until a wheel
  artifact is produced for this release.
- [x] Resolve release hygiene: stale examples README entries, oversized visual
  artifact policy, and local git/workflow push boundaries.

## Agent Working Order

Work in this order unless a later owner records a concrete reason to deviate.

1. Close mathematical gaps: multiscale stability, malformed distances,
   relabelling invariance, transport audit, spectral proxy.
2. Close runtime gaps: dense budgets, sparse/chunked semantics, transport
   scaling diagnostics.
3. Close application gaps: examples for all admitted record families and
   density-vs-Redif disagreement.
4. Close visual/export gaps: native Redif evidence export, visual inspection,
   screenshots, no client-side recomputation.
5. Close documentation and vocabulary gates.
6. Run release gates and record exact command output in this file.

## Mathematical Hardening

### M1. Stability Certificates

Status: [x]
Owner: Codex
Evidence: `metric/modify/dynamics/redif.hpp`;
`tests/core_smoke/redif_stability_smoke.cpp`;
`ctest --test-dir build/core -R 'redif_.*_smoke' --output-on-failure` passed
8/8 on 2026-06-26.

Tasks:

- [x] Add a `RedifStabilityDiagnostics` result block.
- [x] Report pivot margins or condition estimates for implicit Euler solves.
- [x] Report singularity margin for `redif_inverse_noise_step`.
- [x] Classify each inverse request as stable, marginal, singular, or rejected.
- [x] Add thresholds that are explicit options, not hidden constants.
- [x] Add tests where the two-point space becomes singular at `tau = 1/2`.
- [x] Add tests where near-singular systems are accepted only with a warning
  status or refused by the configured margin.

### M2. Multiscale Redif Path Functional

Status: [x]
Owner: Codex
Evidence: `redif_multiscale_transport_paths` in
`metric/modify/dynamics/redif.hpp`;
`tests/core_smoke/redif_multiscale_path_smoke.cpp`;
`docs/concepts/redif-metric-dynamics.md`; focused Redif/visual CTest passed
10/10 on 2026-06-26.

Tasks:

- [x] Define a multiscale Redif run over a grid of `neighbors`, `euler_step`,
  `iterations`, and fixed/adaptive geometry.
- [x] Define aggregate path scores with median, maximum, and
  stability-weighted variants.
- [x] Preserve per-scale scores for auditability.
- [x] Add canonical tests showing stable ranking of isolated atoms and bridge
  atoms across reasonable scale ranges.
- [x] Document when rankings are parameter-sensitive instead of hiding that
  fact.

### M3. Kernel And Local-Scale Axiomatics

Status: [x]
Owner: Codex
Evidence: `metric/modify/dynamics/redif.hpp`;
`tests/core_smoke/redif_operator_kernel_smoke.cpp`;
`docs/concepts/redif-metric-dynamics.md`; focused Redif/visual CTest passed
10/10 on 2026-06-26.

Tasks:

- [x] Document required kernel properties: nonnegative, symmetric after local
  symmetrization, distance-decreasing, finite, and compatible with row
  normalization.
- [x] Make local scale policy inspectable in `RedifOperator`.
- [x] Add scale policies: mean local distance, median local distance, kth local
  distance, and global scale.
- [x] Add canonical tests proving larger local distance yields weaker affinity
  under each admitted policy.
- [x] Add failure tests for zero, negative, non-finite, asymmetric, and
  malformed distance matrices.
- [x] Avoid exposing arbitrary external kernels until the finite metric-space
  contract for them is explicit.

### M4. Entropy Laws

Status: [x]
Owner: Codex
Evidence: `RedifStepDiagnostics` in `metric/modify/dynamics/redif.hpp`;
`tests/core_smoke/redif_entropy_laws_smoke.cpp`;
`docs/concepts/redif-metric-dynamics.md`; focused Redif CTest passed 8/8 on
2026-06-26.

Tasks:

- [x] Separate fixed-operator entropy claims from adaptive-geometry claims in
  diagnostics and docs.
- [x] Report stationary measure per step when adaptive geometry is enabled.
- [x] Report Shannon entropy, relative entropy to initial stationary measure,
  and relative entropy to current stationary measure.
- [x] Add tests for uniform stationary spaces where Shannon entropy increases.
- [x] Add tests for nonuniform stationary spaces where Shannon entropy is not
  claimed unconditionally.
- [x] Document the exact finite Markov-kernel conditions behind each entropy
  claim.

### M5. Degenerate And Canonical Spaces

Status: [x]
Owner: Codex
Evidence: `tests/core_smoke/redif_degenerate_spaces_smoke.cpp`;
`tests/core_smoke/redif_canonical_spaces_smoke.cpp`;
`tests/core_smoke/redif_path_outlier_smoke.cpp`; focused Redif/visual CTest
passed 10/10 on 2026-06-26.

Tasks:

- [x] Add fixtures for duplicated atoms with zero distance between distinct
  records.
- [x] Add disconnected component fixtures and document component behavior.
- [x] Add ultrametric fixtures.
- [x] Add nonuniform local-volume fixtures where density and Redif path
  diagnostics disagree.
- [x] Add clique, chain, two-chain bridge, star, cycle, and line-plus-outlier
  fixtures to canonical Redif tests.
- [x] Verify invariance under record relabelling that preserves the full
  distance matrix and path-ranking result.

### M6. Continuous-Time Operator Contract

Status: [x]
Owner: Codex
Evidence: `docs/concepts/redif-metric-dynamics.md`;
`metric/modify/dynamics/redif.hpp`.

Tasks:

- [x] Define the generator used by Redif in one canonical notation.
- [x] State whether the library treats `P - I`, `Q^{-1}L`, or another normalized
  operator as the primary generator in each public function.
- [x] Document relation between explicit forward Euler and implicit inverse
  Euler.
- [x] Add composition tests for small time steps.
- [x] Decide whether matrix exponential or resolvent-based time evolution is a
  future exact finite operator.
- [x] If exact finite-time evolution is promoted later, add a separate operator
  contract instead of changing Euler semantics silently.

### M7. Transport Geometry Audit

Status: [x]
Owner: Codex
Evidence: `RedifTransportDiagnostics` in `metric/modify/dynamics/redif.hpp`;
`docs/concepts/redif-metric-dynamics.md`;
`tests/core_smoke/redif_degenerate_spaces_smoke.cpp`.

Tasks:

- [x] Audit the discrete Wasserstein distance used for measure-path geometry.
- [x] Document its ground metric assumptions and computational complexity.
- [x] Add tests for known transport distances on two-point and three-point
  spaces.
- [x] Keep approximate transport options out of the public contract until exact
  semantics are pinned.
- [x] Report transport solver diagnostics in `RedifMeasureResult`.
- [x] Add stress tests for larger support sizes and record runtime/memory
  behavior.

## Public API And Application Surface

### A1. C++ Result Contract

Status: [x]
Owner: Codex
Evidence: `RedifMeasureResult`, `RedifStepDiagnostics`, `RedifMeasureSummary`,
and `RedifTransportDiagnostics` in `metric/modify/dynamics/redif.hpp`;
focused Redif/visual CTest passed 10/10 on 2026-06-26.

Tasks:

- [x] Add stable structured metadata for kernel policy, local scale policy,
  geometry policy, generator name, and time discretization.
- [x] Add summary helper for terminal mass, top mass atoms, entropy delta, and
  transport path length.
- [x] Add per-step operator diagnostics to include scale range, spectral gap or
  proxy, stability margin, and component count.
- [x] Preserve inspectability of affinity, degree, transition, stationary
  measure, and local distances.

### A2. Python Surface

Status: [x]
Owner: Codex
Evidence: `python/pkg/metric/operators.py`, `python/pkg/metric/spaces.py`,
`python/pkg/metric/strategies.py`, `python/src/exact_scan.cpp`,
`python/tests/core/test_redif_python_surface.py`;
`python -m unittest python.tests.core.test_redif_python_surface python.tests.core.test_revival_api`
passed 64/64 on 2026-06-26.

Tasks:

- [x] Expose Redif through Python without top-level concrete-algorithm clutter.
- [x] Add `Space.add_noise(...)`, `Space.remove_noise(...)`,
  `Space.dynamics(...)`, and `Space.outliers(strategy=TransportPath(...))`.
- [x] Add strategy objects with finite metric-space names:
  `RedifDynamics`, `ForwardDynamics`, `InverseDynamics`, and `TransportPath`.
- [x] Return Python objects with `to_dict()`, `to_numpy()`, and optional
  `to_pandas()` helpers.
- [x] Keep measure paths explicit; do not pretend inverse dynamics reconstructs
  original non-vector records.
- [x] Add Python tests proving the API works for strings, histograms, vectors,
  and mixed records through their metric only.

### A3. Engine Intent Integration

Status: [x]
Owner: Codex
Evidence: `metric.intent.add_noise`, `metric.intent.remove_noise`,
`metric.intent.dynamics`; `find_outliers(..., TransportPath(...))`; Redif dense
budgets and future sparse/chunked exactness boundaries documented in
`docs/concepts/redif-metric-dynamics.md`; Python Redif/Revival tests passed
65/65 on 2026-06-26.

Tasks:

- [x] Decide whether Redif path scoring belongs behind `find_outliers` as a
  promoted singularity strategy.
- [x] Add intent-level documentation for forward dynamics and inverse dynamics.
- [x] Keep `density_filter` separate from Redif in API docs and examples.
- [x] Add runtime-policy support for Redif dense budget, future sparse-provider
  budget, and refusal diagnostics.

### A4. Examples

Status: [x]
Owner: Codex
Evidence: `python/examples/engine/redif_metric_dynamics.py`;
Python Redif/Revival tests passed 65/65 on 2026-06-26.

Tasks:

- [x] Add a small string-space Redif example.
- [x] Add a histogram/transport-space Redif example.
- [x] Add a process-curve Redif example.
- [x] Add a mixed-record Redif example.
- [x] Add an example where density filtering and Redif path length disagree, so
  users see that these are different finite-space principles.
- [x] Add an example for forward noise addition through metric-induced
  dynamics.
- [x] Add all example commands to docs and run them in CI or a documented smoke
  gate.

### A5. Visual And Inspection Surface

Status: [x]
Owner: Codex
Evidence: `examples/engine/redif_metric_dynamics_visual_export.cpp`;
`visual/tools/check-redif-visual-export.mjs`;
`ctest --test-dir build/core -R 'redif_visual_export_validate|example_engine_redif_metric_dynamics_visual_export' --output-on-failure`
passed 2/2 on 2026-06-26. Source/intermediate/terminal screenshots generated
from the native `metric.visual.v1` Redif export:
`docs/examples/assets/redif-metric-dynamics/redif-source-state.png`,
`docs/examples/assets/redif-metric-dynamics/redif-intermediate-state.png`,
and `docs/examples/assets/redif-metric-dynamics/redif-terminal-state.png`.

Tasks:

- [x] Export Redif measure paths to `metric.visual.v1`.
- [x] Add a measure-path view showing mass flow from an atom over time.
- [x] Add entropy trajectory plots.
- [x] Add transport path-length overlays for singularity inspection.
- [x] Add visual checks that no JavaScript computes Redif dynamics from raw
  records; visuals must consume exported native evidence.
- [x] Add screenshots for source, intermediate, and terminal Redif states.
- [x] Add `redif_visual_export_validate` or equivalent release-gate test.

## Scale And Runtime

### S1. Sparse Redif Operator

Status: [x]
Owner: Codex
Evidence: `RedifSparseOperator`, `RedifLocalRelationEntry`,
`redif_sparse_operator_from_exact_local_relation`,
`redif_sparse_operator_from_exact_neighbor_provider`, and
`redif_operator_from_sparse_operator` in `metric/modify/dynamics/redif.hpp`;
`tests/core_smoke/redif_operator_kernel_smoke.cpp`; focused Redif/visual CTest
passed 10/10 on 2026-06-26.

Tasks:

- [x] Define exact sparse semantics before implementing storage.
- [x] Build Redif from a k-neighbour provider without materializing full dense
  distances when policy allows.
- [x] Keep exact dense Redif as the reference implementation.
- [x] Add sparse affinity/transition representation.
- [x] Add parity tests against dense Redif on small spaces.
- [x] Add refusal diagnostics when sparse approximation would invalidate an
  exactness claim.

### S2. Chunked And Sampled Redif

Status: [x]
Owner: Codex
Evidence: `redif_sampled_relation_options`,
`redif_operator_from_sampled_distance_matrix`, and
`redif_remove_noise_from_sampled_distance_matrix` in
`metric/modify/dynamics/redif.hpp`; sampled/chunked exactness boundaries in
`docs/concepts/redif-metric-dynamics.md`; sampled/chunked path diagnostics and
bounded relation-evaluation test in
`tests/core_smoke/redif_metric_dynamics_smoke.cpp`.

Tasks:

- [x] Define chunked Redif semantics before implementation.
- [x] Decide what is approximate: local relation, transport path, adaptive
  geometry, or all of them.
- [x] Add quality diagnostics for sampled/chunked Redif paths.
- [x] Add scale tests proving bounded metric calls below dense all-pairs work.
- [x] Keep approximate Redif explicitly marked non-exact.

### S3. Transport Scaling

Status: [x]
Owner: Codex
Evidence: `RedifTransportDiagnostics` and transport support/budget options in
`metric/modify/dynamics/redif.hpp`; exact and support-truncated transport tests
in `tests/core_smoke/redif_metric_dynamics_smoke.cpp`; exact transport
smoke/stress fixtures in `tests/core_smoke/redif_degenerate_spaces_smoke.cpp`;
transport exactness boundaries in `docs/concepts/redif-metric-dynamics.md`.

Tasks:

- [x] Profile exact Wasserstein path computation.
- [x] Add budget checks specific to measure transport.
- [x] Add sparse/support-truncated measure options after exact behavior is
  tested.
- [x] Define exactness boundaries before discarding any support mass.
- [x] Add diagnostics for discarded mass if any truncation is introduced.

## Testing Gates

### T1. Focused Redif Test Suite

Status: [x]
Owner: Codex
Evidence: `ctest --test-dir build/core -R 'redif_.*_smoke' --output-on-failure`
passed 8/8 on 2026-06-26. `ctest --test-dir build/core -R
'redif_.*_smoke|redif_visual_export_validate|example_engine_redif_metric_dynamics_visual_export'
--output-on-failure` passed 10/10 on 2026-06-26. `python -m unittest
python.tests.core.test_redif_python_surface python.tests.core.test_revival_api`
passed 65/65 on 2026-06-26.

Required tests:

- [x] `redif_metric_dynamics_smoke`
- [x] `redif_canonical_spaces_smoke`
- [x] `redif_path_outlier_smoke`
- [x] `redif_operator_kernel_smoke`
- [x] `redif_stability_smoke`
- [x] `redif_entropy_laws_smoke`
- [x] `redif_multiscale_path_smoke`
- [x] `redif_degenerate_spaces_smoke`
- [x] `redif_python_surface_test`
- [x] `redif_visual_export_validate`

### T2. Property-Style Invariants

Status: [x]
Owner: Codex
Evidence: focused Redif CTest passed 8/8 on 2026-06-26; individual invariants
are covered by the Redif smoke tests listed above. Focused Redif/visual CTest
passed 10/10 after detailed-balance, relabelling, sparse parity, transport
budget, support truncation, and sampled/chunked diagnostics additions.

Tasks:

- [x] Row stochasticity: every transition row sums to one.
- [x] Probability preservation: every measure remains nonnegative and sums to
  one.
- [x] Symmetric affinity for symmetric local relation.
- [x] Detailed balance for fixed symmetric affinity.
- [x] Relabelling invariance for full measure paths.
- [x] Dense/sparse parity on small spaces.
- [x] Refusal before first metric call when dense budget is exceeded.

### T3. Full Release Verification

Status: [x]
Owner: Codex
Evidence: `cmake --build build/core -j2` completed on 2026-06-26.
`ctest --test-dir build/core --output-on-failure` passed 202/202 on
2026-06-26 after the Redif sparse/sampled/transport changes, with the existing
MNIST example guards skipping tests 159-161. Focused Redif/visual CTest passed
10/10. Python Redif/Revival tests passed 65/65. Full `git diff --check`
passed. Vocabulary scan was reviewed with assets and generated JSON excluded;
remaining hits are explicit comparison tables, lower-level implementation
adapters, roadmap text, or the scan command itself. Python root export smoke
reported `dbscan False` and `kmedoids False`.

Tasks:

- [x] Run full core CTest.
- [x] Run Python source tests for the promoted API.
- [x] Run Python wheel smoke if a wheel is produced for this release; no wheel
  artifact was produced in this source-tree verification, so the source smoke is
  the active gate.
- [x] Run examples that are added under A4.
- [x] Run visual/export validation if A5 lands.
- [x] Run `git diff --check`.
- [x] Record exact commands and output summaries in this file.

## Documentation Gates

### D1. Concept Documentation

Status: [x]
Owner: Codex
Evidence: `docs/concepts/redif-metric-dynamics.md`.

Tasks:

- [x] Update `docs/concepts/redif-metric-dynamics.md` whenever operator
  semantics change.
- [x] Add a dedicated stability section.
- [x] Add a dedicated multiscale section.
- [x] Add diagrams for measure paths and inverse dynamics.
- [x] Keep density filtering and Redif separated.
- [x] Add a parameter-sensitivity section for multiscale rankings.

### D2. API Documentation

Status: [x]
Owner: Codex
Evidence: `docs/api/cpp.md`, `docs/api/python.md`,
`python/examples/engine/redif_metric_dynamics.py`.

Tasks:

- [x] Document C++ Redif APIs in `docs/api/cpp.md`.
- [x] Document Python Redif APIs after the Python surface exists.
- [x] Add examples that start from finite metric spaces, not vector feature
  tables.
- [x] Keep concrete solver or algorithm names out of top-level docs unless they
  are explicitly framed as implementation details or comparison terms.

### D3. Vocabulary Review

Status: [x]
Owner: Codex
Evidence: Vocabulary scan run on 2026-06-26 with assets excluded. Remaining
hits are explicit comparison tables, lower-level adapter names, scale-policy
roadmap text, or the scan command itself. `metric.dbscan` and `metric.kmedoids`
are not top-level exports.

Tasks:

- [x] Scan public docs and Python root exports for inappropriate
  machine-learning vocabulary.
- [x] Keep `dbscan`, `kmedoids`, and comparable concrete procedures outside the
  top-level Python namespace.
- [x] Ensure density filtering is not described as generic denoising.
- [x] Ensure Redif is described as metric-induced inverse dynamics, not an
  ad-hoc outlier or clustering procedure.
- [x] Add a comparison appendix only where comparison to classical procedures is
  useful and explicitly marked as specialization/comparison.

Suggested scan:

```bash
rg -n 'denoise|dbscan|kmedoids|classifier|training|epoch|feature|autoencoder' \
  metric/modify metric/engine.hpp python/pkg docs/api docs/engine docs/concepts docs/examples \
  --glob '!**/assets/**' --glob '!**/*.json'
```

## Repository And Release Hygiene

### H1. Local Change Boundary

Status: [x]
Owner: Codex
Evidence: Dirty worktree still contains many unrelated pre-existing changes, so
no unrelated file was reverted and no staging was performed without an explicit
commit request. `git status --short -- .github .github/workflows` reported no
workflow files in this slice. Redif/dynamics files touched by this plan are
identifiable by the Redif plan, Redif implementation/tests, Redif docs, Python
diagnostic binding/test, and Redif visual screenshot assets.

Tasks:

- [x] Separate Redif/dynamics changes from unrelated dirty worktree changes.
- [x] Do not revert user changes.
- [x] Stage only the intended production-plan and Redif-related files when a
  commit is requested.
- [x] If workflow files are modified, push them only with credentials that have
  the required GitHub workflow scope.

### H2. Stale Documentation References

Status: [x]
Owner: Codex
Evidence: `examples/README.md` no longer links the removed historical example
folders as active shipped folders; targeted stale-folder scan reviewed on
2026-06-26.

Tasks:

- [x] Update `examples/README.md` references that still point at removed example
  folders such as `dnn_examples`, `ensemble_examples`, and `mapping_examples`.
- [x] Re-run documentation link checks or a targeted `rg` scan for stale example
  paths.

### H3. Large Visual Artifact Policy

Status: [x]
Owner: Codex
Evidence: Large evidence artifact policy added to
`docs/visual/reports/native-exporter-quality-audit.md`; current mapping
artifact is 39 MB in this worktree; Redif visual export is generated build
output and small.

Tasks:

- [x] Resolve the large `metric.visual.json` warning for visual assets around
  mapping dimensionality.
- [x] Choose one policy: reduce the file, split it, move it to LFS, or treat it
  as a generated artifact outside the normal source tree.
- [x] Document the policy so future native evidence exports do not repeat the
  same release problem.

## Release Gates

Redif dynamics and noise treatment can be considered production-complete only
when all of these gates are checked:

- [x] Stability diagnostics exist and are tested.
- [x] Multiscale Redif path scoring is specified or explicitly deferred.
- [x] Degenerate canonical spaces are tested.
- [x] Python surface exists or release notes clearly state C++-only support.
- [x] Visual/export evidence exists or release notes clearly state no visual
  support yet.
- [x] Dense budget refusal and future sparse/chunked policies are documented.
- [x] Full CTest passes.
- [x] Python source or wheel tests pass if Python Redif is exposed.
- [x] Public docs avoid machine-learning vocabulary except in explicit
  comparison sections.
- [x] Examples cover non-vector finite metric spaces.
- [x] Release hygiene items H1-H3 are closed or explicitly deferred.

## Acceptance Commands

Record exact local or CI output in the relevant `Evidence` fields.

```bash
cmake --build build/core --target \
  redif_metric_dynamics_smoke \
  redif_canonical_spaces_smoke \
  redif_path_outlier_smoke \
  redif_operator_kernel_smoke \
  redif_stability_smoke \
  redif_entropy_laws_smoke \
  redif_multiscale_path_smoke \
  redif_degenerate_spaces_smoke

ctest --test-dir build/core -R 'redif_.*_smoke' --output-on-failure

ctest --test-dir build/core -R \
  'redif_.*_smoke|redif_visual_export_validate|example_engine_redif_metric_dynamics_visual_export' \
  --output-on-failure

ctest --test-dir build/core --output-on-failure

cmake --build build/core -j2

cmake --build build/python --target metric

PYTHONPATH=python/pkg python/.venv/bin/python -m unittest \
  python.tests.core.test_redif_python_surface \
  python.tests.core.test_revival_api

rg -n 'denoise|dbscan|kmedoids|classifier|training|epoch|feature|autoencoder' \
  metric/modify metric/engine.hpp python/pkg docs/api docs/engine docs/concepts docs/examples \
  --glob '!**/assets/**' --glob '!**/*.json'

PYTHONPATH=python/pkg python/.venv/bin/python - <<'PY'
import metric
for name in ("dbscan", "kmedoids"):
    print(name, hasattr(metric, name))
PY

git diff --check
```

If Python Redif or visual export is not implemented in the target release,
replace the corresponding acceptance command with an explicit release-note entry
that states the current support boundary.

## Agent Progress Log

Append dated entries here:

- 2026-06-26: Created initial Redif dynamics and noise plan from the current
  implemented baseline and remaining mathematical/application gaps.
- 2026-06-26: Added status legend, agent working order, and acceptance commands
  so the file can be used as an editable execution plan.
- 2026-06-26: Implemented Redif stability diagnostics, scale-policy inspection,
  distance-matrix validation, multiscale transport-path scoring, result
  summaries, step diagnostics, transport diagnostics, and five focused Redif
  smoke tests. Focused Redif CTest passed 8/8.
- 2026-06-26: Added native Python Redif bindings, Python strategy/result
  surface, `Space.add_noise`, `Space.remove_noise`, `Space.dynamics`,
  `TransportPath` outlier routing, API documentation, and
  `test_redif_python_surface`. Python Redif/Revival tests passed 64/64.
- 2026-06-26: Reworked this file into a production-readiness TODO plan with an
  open TODO dashboard, mathematical hardening tasks, API/application tasks,
  scale/runtime tasks, documentation gates, test gates, repository hygiene, and
  release gates.
- 2026-06-26: Added Redif spectral proxy diagnostics, detailed-balance checks,
  stronger multiscale canonical tests, parameter-sensitivity documentation,
  full measure-path relabelling invariance, exact transport stress evidence,
  Python diagnostic coverage, finite metric-space Redif examples, native
  `metric.visual.v1` Redif export, and `redif_visual_export_validate`.
  Focused Redif/visual CTest passed 10/10; Python Redif/Revival tests passed
  65/65.
- 2026-06-26: Ran full native release verification:
  `ctest --test-dir build/core --output-on-failure` passed 202/202, with the
  existing MNIST example guards skipping tests 159-161. Full `git diff --check`
  passed. Python root export smoke confirmed `metric.dbscan` and
  `metric.kmedoids` are not exported from the package root. Wheel smoke remains
  not applicable until a wheel artifact is produced for this release.
- 2026-06-26: Completed the remaining Redif dynamics/noise plan items: exact
  sparse local-relation provider and sparse operator diagnostics; dense/sparse
  parity tests; non-exact sampled/chunked local-relation path diagnostics and
  bounded relation-evaluation test; transport problem budgets; support
  truncation with discarded-mass diagnostics; source/intermediate/terminal Redif
  screenshots under `docs/examples/assets/redif-metric-dynamics/`; C++/Python
  docs. `cmake --build build/core -j2` completed, full CTest passed 202/202,
  Python Redif/Revival tests passed 65/65, full `git diff --check` passed, and
  vocabulary/root-export checks were reviewed.
