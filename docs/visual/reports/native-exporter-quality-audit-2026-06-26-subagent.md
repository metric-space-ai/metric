# Native Exporter Quality Audit - 2026-06-26 Subagent

Branch audited: `codex/visual-engine-evidence-exporters`

Scope: native `examples/engine/*visual_export.cpp` exporters, their generated
`metric.visual.v1` documents, and published
`docs/examples/assets/*/metric.visual.json` assets. I did not edit exporters,
runtime, examples, site, tests, or existing report files. Temporary export
outputs were written under `/tmp/metric-native-export-audit.w1OyJB` and default
stdout tests under `/tmp/metric-native-export-default.HUMcbk`.

## Resolution Status

The blocking exporter-contract findings from this audit have been addressed in
the current branch:

- Process-Curve, Solver-Trace and Redif now emit complete `metric.visual.v1`
  JSON to stdout by default and write files only through explicit output
  arguments.
- Redif now carries native, non-synthetic provenance and has a published
  `docs/examples/assets/redif-metric-dynamics/metric.visual.json` asset.
- `visual/tools/check-redif-visual-export.mjs` now rejects synthetic Redif
  evidence and requires explicit native provenance.
- Relation-Matrix public evidence was regenerated from the native exporter and
  is byte-consistent with stdout regeneration. The reproducibility details are
  recorded in
  `docs/visual/reports/relation-matrix-reproducibility-audit.md`.

The remaining process-curve item is not an exporter-contract failure; it is the
known source-record scale blocker for public hero promotion.

## Executive Summary

Blocking issues:

- `redif_metric_dynamics_visual_export.cpp` fails the provenance contract:
  generated JSON contains `provenance.synthetic: true`, does not set
  `provenance.native_export: true`, and has no published
  `docs/examples/assets/redif-metric-dynamics/metric.visual.json`.
- `process_curve_external_visual_export.cpp`, `solver_trace_visual_export.cpp`,
  and `redif_metric_dynamics_visual_export.cpp` do not emit
  `metric.visual.v1` JSON to stdout by default. They write files and print
  status text instead.
- `process-curve-external` remains below the current hero scale threshold:
  64 total records, 48 relation/source records, minimum 500.

Non-blocking integration risks:

- `relation-matrix` regenerated successfully and validates, but the generated
  document differs from the published asset in numeric precision of time-series
  payloads. Counts and references match.
- `mapping-dimensionality` byte-differs from the published asset, but parsed
  canonical JSON is identical; this appears to be formatting only.
- `mapping-dimensionality`, `mixed-records`, and `dynamics-noise` are large
  single-file exporters, which is maintainable for current evidence but worth
  watching before adding more branches.

## Audit Matrix

| Exporter | Source | Published asset | Schema / index | Default stdout | `--export-dir` | Provenance | Internal consistency | Scope |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| condition-monitoring | `examples/engine/condition_monitoring_visual_export.cpp` | `docs/examples/assets/condition-monitoring/metric.visual.json` | PASS | PASS | PASS | PASS | PASS | WARN: 836 LOC |
| cross-space-dependency | `examples/engine/cross_space_dependency_visual_export.cpp` | `docs/examples/assets/cross-space-dependency/metric.visual.json` | PASS | PASS | PASS | PASS | PASS | WARN: 883 LOC |
| dynamics-noise | `examples/engine/finite_metric_dynamics_visual_export.cpp` | `docs/examples/assets/dynamics-noise/metric.visual.json` | PASS | PASS | PASS | PASS | PASS | WARN: 1076 LOC |
| mapping-dimensionality | `examples/engine/mapping_dimensionality_visual_export.cpp` | `docs/examples/assets/mapping-dimensionality/metric.visual.json` | PASS | PASS | PASS | PASS | PASS | WARN: 1492 LOC |
| mixed-records | `examples/engine/mixed_finite_records_visual_export.cpp` | `docs/examples/assets/mixed-records/metric.visual.json` | PASS | PASS | PASS | PASS | PASS | WARN: 1316 LOC |
| process-curve-external | `examples/engine/process_curve_external_visual_export.cpp` | `docs/examples/assets/process-curve-external/metric.visual.json` | PASS | FAIL | PASS | PASS | WARN | PASS: 796 LOC |
| redif-metric-dynamics | `examples/engine/redif_metric_dynamics_visual_export.cpp` | missing | PASS for generated `/tmp` doc | FAIL | PASS | FAIL | PASS | PASS: 261 LOC |
| relation-matrix | `examples/engine/relation_matrix_visual_export.cpp` | `docs/examples/assets/relation-matrix/metric.visual.json` | PASS | PASS | PASS | PASS | WARN | PASS: 756 LOC |
| solver-trace | `examples/engine/solver_trace_visual_export.cpp` | `docs/examples/assets/solver-trace/metric.visual.json` | PASS | FAIL | PASS | PASS | PASS | PASS: 563 LOC |

## condition-monitoring

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Uses native process metric/catalog code in the exporter and `metric/engine.hpp`; no Python/JS algorithm path. |
| Avoids Python/JavaScript algorithm work | PASS | Source has no Python/Node dependency; visual document provenance marks native export. |
| Avoids extra dependencies | PASS | Standard library, METRIC headers, and `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | PASS | Default stdout test parsed `schema: metric.visual.v1`, 22,732,485 bytes. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/condition-monitoring/metric.visual.json`. |
| Provenance native and not synthetic | PASS | `native_export: true`, `synthetic_js: false`; no `synthetic: true`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | PASS | 528 records, 2 relations, 1 space, 2 coordinate sets, 10 full-record properties, 2 diagnostics. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | WARN | 836 LOC, still localized but sizeable. |

Published asset matches the generated output byte-for-byte.

## cross-space-dependency

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Uses native paired data, distance matrices, bridge evidence, and dependence diagnostics. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only; no Python/JS algorithm dependency. |
| Avoids extra dependencies | PASS | Standard library plus local `cross_space_dependency.hpp` and `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | PASS | Default stdout test parsed `schema: metric.visual.v1`, 15,753,238 bytes. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/cross-space-dependency/metric.visual.json`. |
| Provenance native and not synthetic | PASS | `native_export: true`, `synthetic_js: false`; no `synthetic: true`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | PASS | 512 records, 3 relations, 2 spaces, 2 coordinate sets, 6 properties, 4 diagnostics. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | WARN | 883 LOC. |

Published asset matches the generated output byte-for-byte.

## dynamics-noise

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Uses native finite metric dynamics fixture and METRIC transition/diffusion/reconstruction path. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only. |
| Avoids extra dependencies | PASS | Standard library, METRIC headers, local fixture, `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | PASS | Default stdout test parsed `schema: metric.visual.v1`, 15,313,912 bytes. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/dynamics-noise/metric.visual.json`. |
| Provenance native and not synthetic | PASS | JSON has `native_export: true`, `synthetic_js: false`, `native_checks_pass: true`; no `synthetic: true`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | PASS | 512 records, 2 dense relations, 48 coordinate states, 12 properties, 25 passing native diagnostics. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | WARN | 1076 LOC; large but cohesive. |

Published asset matches the generated output byte-for-byte.

## mapping-dimensionality

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Native process-curve metric, diffusion target, parametric coordinate fit, and diagnostics are built in C++. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only. |
| Avoids extra dependencies | PASS | Standard library plus `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | PASS | Default stdout test parsed `schema: metric.visual.v1`, 41,772,247 bytes. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/mapping-dimensionality/metric.visual.json`. |
| Provenance native and not synthetic | PASS | `native_export: true`, `runtime: native C++`, `synthetic_js: false`; no `synthetic: true`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | PASS | 1000 records, 2 spaces, 2 relations, 3 coordinate sets, 9 full-record properties, 4 diagnostics. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | WARN | 1492 LOC; largest exporter in this audit. |

Published asset byte-differs from generated output, but parsed canonical JSON is
identical. This is a formatting-only reproducibility warning.

## mixed-records

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Uses native mixed finite record fixture, metric, nearest-neighbor/contribution, representative, and clustering checks. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only. |
| Avoids extra dependencies | PASS | Standard library, METRIC headers, local fixture/helpers, `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | PASS | Default stdout test parsed `schema: metric.visual.v1`, 7,029,812 bytes. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/mixed-records/metric.visual.json`. |
| Provenance native and not synthetic | PASS | `native_export: true`, `runtime: native C++`, `synthetic_js: false`; no `synthetic: true`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | PASS | 2000 records, 1 relation, 1 space, 2 coordinate sets, 11 full-record properties, 10 ok diagnostics. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | WARN | 1316 LOC; broad but still one domain. |

Published asset matches the generated output byte-for-byte.

## process-curve-external

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Loads CSV inputs and computes aligned metric, baseline winners, margins, and diagnostics in C++. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only. |
| Avoids extra dependencies | PASS | Standard library, METRIC headers, compiled CSV paths, `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | FAIL | Default run wrote `docs/examples/assets/process-curve-external/metric.visual.json` relative to cwd and printed status text; stdout was not JSON. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/process-curve-external/metric.visual.json`. |
| Provenance native and not synthetic | PASS | `native_export: true`, `synthetic: false`, native CSV source fields. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | WARN | Schema references are valid, but properties intentionally target subsets: 48 source records or 16 query records out of 64 total. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | PASS | 796 LOC. |

Published asset matches the generated output byte-for-byte. The exporter is
schema-valid but does not meet the default stdout contract. It is also the only
native public asset currently blocked by `check-native-hero-evidence-scale`
because the scale count is 48 relation/source records against a 500-record
minimum.

## redif-metric-dynamics

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Calls `mtrc::redif_remove_noise` and `mtrc::redif_add_noise` in C++. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only; Redif-specific checker confirms visual JS has no Redif operator implementation. |
| Avoids extra dependencies | PASS | Standard library, METRIC headers, `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | FAIL | Default run wrote `redif_visual_export/metric.visual.json` relative to cwd and printed status text; stdout was not JSON. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/redif-metric-dynamics/metric.visual.json`. |
| Provenance native and not synthetic | FAIL | Generated doc has `provenance.synthetic: true` and no `native_export: true`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | PASS | 7 records, 1 relation, 1 space, 5 coordinate steps, 2 properties, 1 diagnostic; references validate. |
| `check-visual-document` passes | PASS for generated `/tmp` doc | It passes schema/indexing but reports `synthetic: true`. |
| Source scope | PASS | 261 LOC. |

There is no published
`docs/examples/assets/redif-metric-dynamics/metric.visual.json`; only PNG/SVG
assets exist in that directory. `node visual/tools/check-redif-visual-export.mjs`
passes on the generated `/tmp` document, but that special gate does not cover
the provenance contract failure.

## relation-matrix

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Builds process-curve records, aligned metric relation, block layout, graph, and diagnostics in C++. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only. |
| Avoids extra dependencies | PASS | Standard library plus `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | PASS | Default stdout test parsed `schema: metric.visual.v1`, 760,738 bytes. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/relation-matrix/metric.visual.json`. |
| Provenance native and not synthetic | PASS | `native_export: true`, `synthetic: false`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | WARN | References and counts pass, but generated payload numeric precision differs from published asset. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | PASS | 756 LOC. |

The generated output differs from the published asset in time-series payload
precision, for example `0.02402484181644084` generated vs `0.0240248`
published. Counts, IDs, references, and validation results match. Parent should
decide whether to publish current full-precision output or intentionally keep
rounded payloads.

## solver-trace

Pass/fail:

| Check | Result | Evidence |
| --- | --- | --- |
| Computes evidence in C++ | PASS | Builds native metric-Laplacian/Jacobi solver trace, residual/objective diagnostics, graph, timeline, and properties. |
| Avoids Python/JavaScript algorithm work | PASS | C++ exporter only. |
| Avoids extra dependencies | PASS | Standard library plus `mtrc_visual.hpp`. |
| Emits `metric.visual.v1` by default | FAIL | Default run wrote `docs/examples/assets/solver-trace/metric.visual.json` relative to cwd and printed status text; stdout was not JSON. |
| `--export-dir` writes `metric.visual.json` | PASS | Wrote `/tmp/metric-native-export-audit.w1OyJB/solver-trace/metric.visual.json`. |
| Provenance native and not synthetic | PASS | `native_export: true`, `synthetic_js: false`, `native_checks_pass: true`; no `synthetic: true`. |
| Records/relations/spaces/coordinates/properties/diagnostics consistent | PASS | 12 records, 1 relation, 1 space, 1 coordinate set, 6 properties, 2 passing diagnostics. |
| `check-visual-document` passes | PASS | Published and generated docs pass. |
| Source scope | PASS | 563 LOC. |

Published asset matches the generated output byte-for-byte. The remaining issue
is the CLI default behavior.

## Docs Asset Validation Summary

| Asset | Records | Relations | Spaces | Coordinates | Properties | Diagnostics | Native | Synthetic | Result |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- |
| `condition-monitoring` | 528 | 2 | 1 | 2 | 10 | 2 | true | false | PASS |
| `cross-space-dependency` | 512 | 3 | 2 | 2 | 6 | 4 | true | false | PASS |
| `dynamics-noise` | 512 | 2 | 1 | 48 | 12 | 25 | true | false | PASS |
| `mapping-dimensionality` | 1000 | 2 | 2 | 3 | 9 | 4 | true | false | PASS |
| `mixed-records` | 2000 | 1 | 1 | 2 | 11 | 10 | true | false | PASS |
| `process-curve-external` | 64 | 1 | 1 | 1 | 11 | 1 | true | false | PASS |
| `relation-matrix` | 130 | 1 | 1 | 1 | 7 | 1 | true | false | PASS |
| `solver-trace` | 12 | 1 | 1 | 1 | 6 | 2 | true | false | PASS |

No published Redif `metric.visual.json` exists to validate.

## Recommended Parent Integration Steps

1. Fix Redif provenance and publication before treating it as native public
   evidence: set `native_export: true`, remove or set `synthetic: false`, add
   `synthetic_js: false`, use a source-path writer/generator value, and publish
   `docs/examples/assets/redif-metric-dynamics/metric.visual.json` after
   validation.
2. Normalize CLI behavior for `process_curve_external_visual_export.cpp`,
   `solver_trace_visual_export.cpp`, and
   `redif_metric_dynamics_visual_export.cpp`: no args should emit JSON to
   stdout; `--export-dir <dir>` should be the file-writing path.
3. Refresh or intentionally preserve the `relation-matrix` published asset
   after deciding whether generated full-precision payloads are desired.
4. Clarify `process-curve-external` property target semantics. The current
   partial source/query property sets are valid but differ from other exporters'
   full-record property convention.
5. Keep large exporters on a refactor watchlist. Do not refactor as part of the
   contract fix unless touching the same code anyway.

## Commands and Results

Repository orientation:

```sh
pwd
git branch --show-current
git status --short
sed -n '1,240p' docs/visual/agent-tasks/exporter-quality-audit.md
find docs/examples/assets -name metric.visual.json -print | sort
```

Results: branch was `codex/visual-engine-evidence-exporters`; workspace was
already dirty, including the existing
`docs/visual/reports/native-exporter-quality-audit.md`; eight published
`metric.visual.json` assets were present; Redif asset directory had PNG/SVG
files but no `metric.visual.json`.

Build command used:

```sh
cmake --build build/core --target \
  engine_condition_monitoring_visual_export \
  engine_cross_space_dependency_visual_export \
  engine_finite_metric_dynamics_visual_export \
  engine_mapping_dimensionality_visual_export \
  engine_mixed_finite_records_visual_export \
  engine_process_curve_external_visual_export \
  engine_redif_metric_dynamics_visual_export \
  engine_relation_matrix_visual_export \
  engine_solver_trace_visual_export -- -j4
```

Result: exit 0. Several targets rebuilt in `build/core`; linker emitted duplicate
library warnings for `-ldl`/`-lm`, but all targets built.

Temporary `--export-dir` run command pattern:

```sh
build/core/examples/engine/<exporter-binary> --export-dir /tmp/metric-native-export-audit.w1OyJB/<asset-name>
```

Results:

| Export | Result |
| --- | --- |
| condition-monitoring | wrote `metric.visual.json`, 22,732,485 bytes |
| cross-space-dependency | wrote `metric.visual.json`, 15,753,238 bytes |
| dynamics-noise | wrote `metric.visual.json`, 15,313,912 bytes |
| mapping-dimensionality | wrote `metric.visual.json`, 41,772,247 bytes |
| mixed-records | wrote `metric.visual.json`, 7,029,812 bytes |
| process-curve-external | wrote `metric.visual.json`, 411,768 bytes |
| redif-metric-dynamics | wrote `metric.visual.json`, 19,104 bytes |
| relation-matrix | wrote `metric.visual.json`, 760,738 bytes |
| solver-trace | wrote `metric.visual.json`, 32,240 bytes |

Validation commands:

```sh
node visual/tools/check-visual-document.mjs docs/examples/assets/<asset>/metric.visual.json
node visual/tools/check-visual-document.mjs /tmp/metric-native-export-audit.w1OyJB/<asset>/metric.visual.json
node visual/tools/check-redif-visual-export.mjs /tmp/metric-native-export-audit.w1OyJB/redif-metric-dynamics/metric.visual.json
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
```

Results: all eight published assets passed `check-visual-document`; all nine
generated `/tmp` documents passed `check-visual-document`; Redif special checker
passed 11/11 checks; `check-native-hero-evidence-scale` returned `ok: true` with
6/7 native scale-ready rows and `process-curve-external-hero` blocked by record
count; `check-public-gallery-evidence` returned `ok: true`.

Default stdout command pattern:

```sh
(cd /tmp/metric-native-export-default.HUMcbk/<asset> && /Users/michaelwelsch/Documents/metric/build/core/examples/engine/<exporter-binary> >stdout.log 2>stderr.log)
```

Results:

| Export | Stdout JSON? | Side-effect files under temp cwd |
| --- | --- | --- |
| condition-monitoring | PASS | none |
| cross-space-dependency | PASS | none |
| dynamics-noise | PASS | none |
| mapping-dimensionality | PASS | none |
| mixed-records | PASS | none |
| process-curve-external | FAIL | `docs/examples/assets/process-curve-external/metric.visual.json` |
| redif-metric-dynamics | FAIL | `redif_visual_export/metric.visual.json` |
| relation-matrix | PASS | none |
| solver-trace | FAIL | `docs/examples/assets/solver-trace/metric.visual.json` |

Custom consistency checks:

```sh
node -e '<record/relation/space/coordinate/property reference audit>'
node -e '<published-vs-generated JSON comparison>'
```

Results: no unknown IDs, duplicate coordinate IDs, or coordinate/space count
mismatches were found. `process-curve-external` has partial record-property
coverage by design. Published-vs-generated comparison showed exact byte matches
for condition-monitoring, cross-space-dependency, dynamics-noise,
mixed-records, process-curve-external, and solver-trace; canonical equality for
mapping-dimensionality; numeric precision differences for relation-matrix.
