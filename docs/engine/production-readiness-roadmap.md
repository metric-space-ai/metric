# METRIC Production Readiness Roadmap

Date: 2026-06-25
Status: Production ready for the current verified snapshot
Owner: unassigned
Last local verification: 2026-06-25, `cmake --build build/core -j2` passed;
`ctest --test-dir build/core --output-on-failure` passed with 190 executed
tests and 3 MNIST IDX-data skips after the coordinate-solver facade and promoted
example refactor. After the release-readiness docs update, `git diff --check`,
`ruby scripts/check_markdown_links.rb`, public vocabulary scans, and tracked
artifact scans passed; generated `output/playwright/` and `output/visual/`
evidence directories are ignored. A clean workspace snapshot in `/tmp` built
from scratch and passed CTest with 190 executed tests and 3 MNIST IDX-data skips.
Real MNIST IDX-backed smoke/gallery/integrity runs passed with temporary IDX
data extracted to `/tmp/metric-mnist-idx`. A clean Python 3.12 wheel smoke passed
from a fresh temporary venv. A temporary committed snapshot (`2d5bcdb`) was
cloned cleanly to `/tmp/metric-release-clean-clone`; configure, build, and CTest
passed from an empty build directory with 190 executed tests and 3 MNIST
IDX-data skips.

This document is the editable progress log for making METRIC production ready as
a library for computing with finite metric spaces. It is not a release note.
Agents working on this roadmap should update the status fields, evidence links,
and blocker notes as work lands.

## Status Legend

- `[ ]` Not started
- `[~]` In progress
- `[x]` Done with evidence
- `[!]` Blocked or needs decision

## Production Definition

METRIC is production ready only when all of the following are true:

- The public model is finite metric spaces, metric-induced dynamics, metric
  measures, representations, and derived spaces.
- Machine-learning terms appear only as implementation details or comparison
  notes, never as the organizing public API.
- Redif is specified and tested as metric-space dynamics and inverse dynamics,
  not as an ad hoc denoising heuristic.
- Noise addition and noise removal are dual operations over the same
  metric-induced dynamics.
- Public C++ and Python APIs have no legacy compatibility import layers.
- Build, C++ smoke tests, Python runtime tests, docs checks, and release gates
  pass on a clean host or CI worker.

## Release Gate Summary

| Gate | Status | Evidence | Blocker / Next Action |
| --- | --- | --- | --- |
| Public finite-metric-space vocabulary | [x] | Refactor landed across `metric/modify`, Python adapters, docs, and promoted Engine examples; local audit now leaves comparison-table/Roadmap terms and explicit density-filter distinction only | Re-run scan after any public API/doc change |
| Legacy compatibility removal | [x] | Removed mapping/compat package surfaces; source and installed-wheel probes report `metric.compat`, `metric.space`, `metric.mapping` absent | Keep wheel smoke as release gate |
| Redif mathematical specification | [x] | `docs/concepts/redif-metric-dynamics.md` | Keep spec aligned as operator contract evolves |
| Redif implementation contract | [x] | `metric/modify/dynamics/redif.hpp`, `redif_operator`, `redif_forward_noise_step`, `redif_inverse_noise_step`, Redif smoke tests | Keep future operator extensions covered by canonical fixtures |
| Noise addition via diffusion | [x] | `redif_add_noise`, shared `redif_operator`, `redif_canonical_spaces_smoke` | Keep entropy claims scoped to checked operator conditions |
| Noise removal via inverse dynamics | [x] | `redif_remove_noise`, `redif_inverse_noise_step`, `redif_canonical_spaces_smoke` | Keep inverse checks scoped to finite operator stability |
| Outlier detection via diffusion paths | [x] | `docs/concepts/redif-metric-dynamics.md`, `redif_path_outlier_smoke` | Keep additional fixtures additive as new spaces are studied |
| Density filtering separated from denoising | [x] | `density_filter` naming and smoke tests | Keep DBSCAN language out of denoising docs |
| Parametric diffusion coordinates | [x] | Modify headers and promoted Engine examples now use the coordinate-solver facade; targeted parametric/diffusion-coordinate examples pass | Keep concrete solver details under `metric/solve/parametric/dnn` |
| Python package runtime | [x] | `scripts/check_python_wheel_smoke.py` builds an isolated Python 3.12 wheel, installs it, verifies package surface, runs core tests and promoted examples | Keep smoke aligned with adapter-only Python surface |
| Full C++ build | [x] | `cmake --build build/core -j2` passed after Redif, diagnostics, coordinate-solver facade, promoted example refactor, and CTest blocker fixes; clean workspace snapshot and clean clone builds also passed | Re-run after further C++ header edits |
| Full CTest | [x] | `ctest --test-dir build/core --output-on-failure`: 193 tests, 190 passed, 3 MNIST skipped due missing IDX data, 0 failed, 45.92s; clean workspace snapshot and clean clone CTests also passed with 190 executed and 3 MNIST skips | Re-run on clean host and after further C++ edits |
| Focused CTests | [x] | Focused Redif/parametric/operator tests passed; isolated blocker regressions now pass | Keep focused set as faster preflight before full CTest |
| Long examples / MNIST | [x] | MNIST targets build and skip cleanly without IDX files; real IDX-backed smoke/gallery/integrity runs passed via `/tmp/metric-mnist-idx` | Re-run on the release host when external IDX data is mounted |
| Release docs | [x] | This roadmap, Redif concept spec, `CHANGELOG.md`, and `docs/release-readiness-notes.md` document release positioning and known limitations | Keep notes current as gates change |

## Research Work

### R1. Formal Redif Dynamics

Status: [x]
Owner:
Evidence: `docs/concepts/redif-metric-dynamics.md`

Tasks:

- [x] Define Redif for a finite metric space `(X, d)`.
- [x] Define the measure used on `X`: atom measures with Dirac initial states.
- [x] Define the metric-induced local affinity and forward transition from distances.
- [x] Define normalization: Redif local Laplacian and row-stochastic Markov transition.
- [x] Define the forward Euler step in finite-dimensional operator form.
- [x] Define inverse dynamics and document current well-posedness limits.
- [x] Define the entropy functional used by the library.
- [x] Document when forward dynamics increases entropy and when only relative entropy is contracted.
- [x] Document why inverse dynamics is metric-derived and where the stability gate remains open.
- [x] Specify failure modes: singular operator, unstable inverse, nonmetric input, disconnected space, degenerate kernel.

Required artifact:

- [x] `docs/concepts/redif-metric-dynamics.md`

### R2. Redif Outlier Functional

Status: [x]
Owner:
Evidence: `docs/concepts/redif-metric-dynamics.md`, `tests/core_smoke/redif_path_outlier_smoke.cpp`

Tasks:

- [x] Define a diffusion path length functional for a record.
- [x] Define why long paths indicate weak participation in metric-space clumping.
- [x] Compare the path functional to local volume, nearest-neighbor isolation, and density-unassigned records.
- [x] Specify invariants under record relabeling.
- [x] Specify behavior on small canonical finite spaces: chain, clique-like cluster, two clusters plus bridge, isolated point.

Required tests:

- [x] `tests/core_smoke/redif_path_outlier_smoke.cpp`
- [x] `tests/core_smoke/redif_canonical_spaces_smoke.cpp`

### R3. Noise As Dynamics

Status: [x]
Owner:
Evidence: `docs/concepts/redif-metric-dynamics.md`, `tests/core_smoke/redif_canonical_spaces_smoke.cpp`

Tasks:

- [x] Define "adding noise" as forward metric-induced dynamics.
- [x] Define "removing noise" as inverse dynamics.
- [x] Ensure both operations share one operator family and one parameterization.
- [x] Document why Gaussian noise is only a special case for particular metrics and coordinate records.
- [x] Add examples over non-vector records and non-Euclidean metrics.

Required tests:

- [x] Forward diffusion increases the selected entropy on a canonical finite space.
- [x] Inverse Redif approximately reverses a bounded forward step.
- [x] Inverse Redif refuses unstable parameters instead of silently exploding.

## API Work

### A1. Public Vocabulary Audit

Status: [x]
Owner:
Evidence: `metric/modify/dynamics/redif.hpp`, `tests/core_smoke/redif_metric_dynamics_smoke.cpp`, `tests/core_smoke/redif_canonical_spaces_smoke.cpp`, `tests/core_smoke/redif_path_outlier_smoke.cpp`

2026-06-25 local audit:

- [x] The old Redif denoise symbol no longer appears in C++ headers, tests, or Redif docs.
- [x] Redif-near docs now use `redif_remove_noise`, inverse dynamics, and forward metric dynamics.
- [x] API/engine/concept/example docs scan now leaves only explicit comparison-table terms, Roadmap checklist text, and the explicit `density_filter` versus Redif distinction.
- [x] Python package scan no longer reports internal `feature_records` or adapter compatibility wording.
- [x] Mixed-record visual asset payload now uses `record_attributes` instead of `features`; native exporter and visual document check pass.
- [x] Public `metric/modify` parametric-coordinate headers now use `solve::parametric::CoordinateSolver`, `CoordinateObjective`, `CoordinateCalibrationReport`, and `CoordinateSolverArtifact`; concrete DNN names are quarantined under `metric/solve/parametric/dnn`.
- [x] Promoted Engine examples now use `make_linear_coordinate_solver`, `make_coordinate_record_codec`, and coordinate-target report helpers instead of direct concrete solver construction or report internals.

Tasks:

- [x] Audit public C++ headers outside `metric/solve/parametric/dnn`.
- [x] Audit Python package public modules.
- [x] Audit docs under `docs/api`, `docs/engine`, `docs/concepts`, and `docs/examples`.
- [x] Permit ML vocabulary only in explicit comparison tables or solver-internal implementation notes.
- [x] Remove or rename public terms: `training`, `epoch`, `feature`, `autoencoder`, `legacy`, `compat`, except explicit comparison tables, Roadmap checklist text, and solver-internal implementation files.
- [x] Keep `metric.spaces` as the finite-space package; do not restore `metric.space`.

Suggested commands:

```bash
rg -n "legacy|compat|training|epoch|feature|autoencoder|PHATE|phate|denoise|noise" \
  metric/modify metric/engine.hpp python/pkg docs/api docs/engine docs/concepts docs/examples
```

### A2. Python Package Surface

Status: [x]
Owner:
Evidence: Python 3.12 source tests with native extensions; package-surface import probe; `scripts/check_python_wheel_smoke.py`

Tasks:

- [x] Build the native Python extensions locally or in CI.
- [x] Verify `import metric` succeeds from a clean install.
- [x] Verify `metric.compat`, `metric.space`, and `metric.mapping` are absent from built wheel.
- [x] Verify `metric.compat`, `metric.space`, and `metric.mapping` are absent from source package import path.
- [x] Verify `metric.spaces`, `metric.mappings`, and `metric.transforms` are the intended public modules from source package import path.
- [x] Run Python runtime tests with native extensions installed on source package path.

Required commands:

```bash
python -m pip wheel . --no-deps -w dist
python -m pip install --force-reinstall dist/*.whl
python -m unittest python.tests.core.test_revival_api
python -m unittest python.tests.core.test_binding_adapter_boundary
python -m unittest python.tests.core.test_mapping_pipeline_adapter
```

### A3. C++ Public Surface

Status: [x]
Owner:
Evidence: `metric/engine.hpp`, `metric/modify/resample/density_filter.hpp`, `metric/solve/parametric/dnn/coordinate_solver.hpp`, vocabulary scan

Tasks:

- [x] Confirm `metric/engine.hpp` exposes finite metric-space operations, not ML algorithm collections.
- [x] Confirm `modify/dynamics` owns Redif and diffusion dynamics.
- [x] Confirm `modify/resample/density_filter.hpp` is not described as denoising.
- [x] Confirm direct `native_autoencoder` mapping is not public.
- [x] Confirm solver-internal DNN names stay under `metric/solve/parametric/dnn`.

## Implementation Work

### I1. Redif Implementation

Status: [x]
Owner:
Evidence: `metric/modify/dynamics/redif.hpp`, `docs/concepts/redif-metric-dynamics.md`, `redif_canonical_spaces_smoke`, `redif_metric_dynamics_smoke`, `redif_path_outlier_smoke`

Tasks:

- [x] Align code with the formal Redif spec.
- [x] Replace any provisional names once the spec chooses final terms.
- [x] Make kernel construction explicit and inspectable.
- [x] Make diffusion operator normalization explicit and inspectable.
- [x] Return diagnostics: entropy before/after, operator scale, step count, path lengths, and stability flags.
- [x] Add preflight checks for unstable inverse dynamics.
- [x] Add exact small-space tests with known expected values.

### I2. Parametric Diffusion Coordinates

Status: [x]
Owner:
Evidence:
`metric/solve/parametric/dnn/coordinate_solver.hpp`, `metric/modify/map/parametric_diffusion_coordinates.hpp`,
`metric/modify/map/parametric_coordinate_artifact.hpp`, `metric/modify/compose/parametric_diffusion_coordinates.hpp`,
promoted Engine example scan, targeted CTest set

Tasks:

- [x] Keep public API terms as coordinate calibration, coordinate solver, coordinate artifact.
- [x] Keep `AutoencoderModel` visible only as a concrete solver type where unavoidable.
- [x] Add a coordinate-solver facade so `metric/modify` public headers do not name concrete DNN solver types.
- [x] Refactor promoted C++ examples that still construct concrete DNN solvers directly, or move them into solver-internal example scope.
- [x] Ensure artifact metadata uses `calibration_spec`, `calibration_steps`, and `diffusion_coordinates`.
- [x] Ensure Python binding arguments use `calibration_steps` and `step_size`.

### I3. Density Filtering

Status: [x]
Owner:
Evidence:

Tasks:

- [x] Rename DBSCAN-based denoise path to density filter.
- [x] Use assigned/unassigned record terminology.
- [x] Re-audit docs after Redif docs land to ensure no DBSCAN denoising language returns.

### I4. Local CTest Blocker Repairs

Status: [x]
Owner:
Evidence: local full CTest, targeted CTests named below

Tasks:

- [x] Materialize PCFA coordinate projection matrix products in `metric/modify/map/pcfa.hpp` so expression objects do not reference temporaries.
- [x] Align standardized-vector metric validation wording with calibration terminology.
- [x] Update pipeline-plan roundtrip expectations to `diffusion_coordinates`.
- [x] Make MNIST examples skip explicitly with return code 77 when required IDX calibration files are absent.
- [x] Rename promoted MNIST reader helpers from training to calibration terminology.

Required tests:

- [x] `ctest --test-dir build/core --output-on-failure -R '^engine_pcfa_mapping_smoke$'`
- [x] `ctest --test-dir build/core --output-on-failure -R '^metric_standardized_smoke$'`
- [x] `ctest --test-dir build/core --output-on-failure -R '^pipeline_plan_smoke$'`
- [x] `ctest --test-dir build/core --output-on-failure -R 'example_engine_mnist_diffusion_coordinate_smoke|example_engine_mnist_diffusion_coordinate_gallery|example_engine_mnist_grae10_integrity'`
- [x] `ctest --test-dir build/core --output-on-failure`

## Verification Work

### V1. C++ Build And Tests

Status: [x]
Owner:
Evidence:

Already observed:

- [x] `cmake -S . -B build/core`
- [x] `cmake --build build/core -j2` after shared Redif operator, diagnostics, coordinate-solver facade, and CTest blocker repairs
- [x] Focused CTest set: 15/15 passed after shared Redif operator, diagnostics, and coordinate-solver facade
- [x] Redif analytical CTest set: 3/3 passed
- [x] Full local CTest: 193 tests discovered, 190 passed, 3 MNIST tests skipped because IDX calibration data is absent, 0 failed, 45.92s
- [x] MNIST tests use `SKIP_RETURN_CODE 77` for missing IDX data instead of aborting
- [x] Targeted promoted parametric/diffusion-coordinate examples: 9/9 CTests passed, with the 2 MNIST IDX-data tests skipped cleanly
- [x] Clean workspace snapshot in `/tmp/metric-clean-snapshot` configured, built, and passed CTest from a fresh build directory: 193 tests discovered, 190 executed tests passed, 3 MNIST IDX-data skips, 0 failed, 63.03s
- [x] Clean clone from temporary committed snapshot `2d5bcdb` configured, built, and passed CTest from a fresh build directory: 193 tests discovered, 190 executed tests passed, 3 MNIST IDX-data skips, 0 failed, 64.15s
- [x] Real IDX-backed MNIST smoke run: 60 records, 6 per class, 3600 dense evaluations, 4 calibration steps, final coordinate-target MSE `0.122599`
- [x] Real IDX-backed MNIST gallery run: 30 records, 3 per class, 900 dense evaluations, 2 calibration steps, final coordinate-target MSE `0.154547`
- [x] Real IDX-backed MNIST GRAE10 integrity run: 60000 visual records, labels match MNIST IDX, 60000 finite p2 coordinates, 60000 finite p3 coordinates

Repeat after future release edits:

- Re-run full `ctest --test-dir build/core --output-on-failure` from a committed clean clone or CI worker.
- [x] Redif analytical tests after formal spec lands.
- [x] Real IDX-backed MNIST smoke/gallery/integrity examples with explicit timeout and expected output capture.

### V2. Python Build And Tests

Status: [x]
Owner:
Evidence: `scripts/check_python_wheel_smoke.py`

Already observed:

- [x] `python3 -m compileall -q python/pkg`
- [x] `PYTHONPATH=python/pkg /Users/michaelwelsch/.local/bin/python3.12 -m unittest python.tests.core.test_revival_api`
- [x] `PYTHONPATH=python/pkg uv run --python /Users/michaelwelsch/.local/bin/python3.12 --with pytest --with numpy python -m pytest python/tests/core/test_binding_adapter_boundary.py python/tests/core/test_mapping_pipeline_adapter.py`
- [x] Source import probe: `metric.compat`, `metric.space`, `metric.mapping` absent; `metric.spaces`, `metric.mappings`, `metric.transforms` present
- [x] `/Users/michaelwelsch/.local/bin/python3.12 scripts/check_python_wheel_smoke.py`
- [x] Clean Python 3.12 wheel smoke from fresh temporary venv: built `mtrc-0.3.4-cp312-cp312-macosx_26_0_arm64.whl`, installed it, verified package surface, ran 55 unittest cases, 13 pytest cases, and all promoted Python examples

Completed requirements:

- [x] Build/install wheel containing native extensions.
- [x] Run Python core tests from installed wheel.
- [x] Confirm no generated `.so` or `__pycache__` files are committed.

### V3. Documentation Checks

Status: [x]
Owner:
Evidence: local markdown link check, Sphinx removed-module scan, PHATE/process-curve scan, comparison-table audit

Tasks:

- [x] Verify all renamed PHATE/process-curve docs have valid links.
- [x] Verify Sphinx docs do not reference removed `metric.space`, `metric.compat`, or `metric.mapping`.
- [x] Verify Redif docs distinguish dynamics, inverse dynamics, density filtering, and outlier path diagnostics.
- [x] Verify comparison table explicitly frames ML methods as special cases, not primary concepts.

## Final Release Checklist

Status: [x]
Owner:
Evidence:

- [x] All gates above are `[x]`.
- [x] Clean clone build passes.
- [x] Clean wheel build passes.
- [x] Public API scan has no blocked terms outside allowed solver internals/comparison docs.
- [x] Redif spec and tests are complete.
- [x] Release notes explain finite metric-space positioning.
- [x] Known limitations are documented.
- [x] No generated build artifacts, `.so` files, or `__pycache__` directories in the commit.
