# Native Visual Exporter Work Queue

This queue is the execution layer for the visual engine plan. It exists so
parallel agents can work on native evidence exporters without touching the same
files.

## Git Discipline

- Work happens on `codex/visual-engine-evidence-exporters`.
- Every integration point gets a commit before the next parallel wave.
- Workers own disjoint source files. The parent agent owns CMake, tests,
  project page integration, and final commits.
- Generated screenshots and runtime outputs are not committed.

## Current Integration Gates

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-views.mjs
node visual/tools/check-visual-regression-public-examples.mjs
node visual/tools/check-visual-performance-large-scenes.mjs
node visual/tools/check-visual-document.mjs <exported metric.visual.json>
node visual/tools/check-redif-visual-export.mjs <redif metric.visual.json>
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
ctest --test-dir build/core -L 'metric_application_evidence|metric_diffusion_coordinate_pipeline|metric_mnist|metric_visual_integrity|metric_benchmark_report' --output-on-failure
```

## Parallel Work Slots

| Slot | Owner writes | Status | Integration target |
| --- | --- | --- | --- |
| Mixed records exporter | `examples/engine/mixed_finite_records_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/mixed-records/metric.visual.json` |
| Cross-space exporter | `examples/engine/cross_space_dependency_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/cross-space-dependency/metric.visual.json` |
| Dynamics exporter | `examples/engine/finite_metric_dynamics_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/dynamics-noise/metric.visual.json` |
| Relation matrix exporter | `examples/engine/relation_matrix_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/relation-matrix/metric.visual.json` |
| Condition monitoring exporter | `examples/engine/condition_monitoring_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/condition-monitoring/metric.visual.json` |
| Mapping/dimensionality exporter | `examples/engine/mapping_dimensionality_visual_export.cpp` | integrated in CMake/CTest | `docs/examples/assets/mapping-dimensionality/metric.visual.json` |
| Process-curve external exporter | `examples/engine/process_curve_external_visual_export.cpp` | integrated in CMake/CTest; stdout/file-output contract normalized; preview-only; blocked for hero scale until more real UCR source windows are available | `docs/examples/assets/process-curve-external/metric.visual.json` |
| Solver-trace exporter | `examples/engine/solver_trace_visual_export.cpp` | integrated in CMake/CTest; stdout/file-output contract normalized | `docs/examples/assets/solver-trace/metric.visual.json` |
| Redif metric dynamics exporter | `examples/engine/redif_metric_dynamics_visual_export.cpp` | integrated in CMake/CTest; native non-synthetic public asset and checker enforced | `docs/examples/assets/redif-metric-dynamics/metric.visual.json` |

## Integrated Exporter Tests

The current branch adds native exporter binaries and validation tests:

```bash
ctest --test-dir build/core -R 'visual_(export|validate)' --output-on-failure
```

The test pair for each exporter first writes `metric.visual.json` from the C++
binary and then validates that document through
`visual/tools/check-visual-document.mjs`. Passing this gate means the exporter is
native, schema-valid, and indexable. It does not mean the corresponding public
hero visualization is finished.

Integrated native exporter coverage now includes mixed records, cross-space
dependency, finite metric dynamics, relation matrix, condition monitoring,
mapping/dimensionality evidence, external UCR process-curve evidence,
solver-trace evidence and Redif metric-dynamics evidence.

`visual/tools/check-native-hero-evidence-scale.mjs` now checks those native
assets directly against the public visual briefs before browser screenshot
review. It verifies native provenance, reports which previews meet native scale
requirements and enforces that missing record-count or record-type diversity is
represented by explicit acceptance blockers rather than silently passing as a
hero candidate. Current state: condition monitoring, mixed records,
cross-space dependency, finite dynamics/noise, relation matrix and
mapping/dimensionality all meet their native scale thresholds. They remain
preview-only until screenshot review accepts the visual grammar and
composition. The external UCR process-curve preview is the only current native
exporter still blocked by source-record count.

The UCR process-curve export has real native evidence, query outcomes and
baseline comparison outside the browser renderer, but it is not yet a scale
candidate for public hero acceptance. The checked-in UCR-derived slices contain
48 real source windows plus 16 query records; the source-source metric relation
and coordinate state cover only those 48 source windows, below the
500-source-record target. Do not scale this hero with synthetic windows or query
padding. The exact current blocker is
`missing-real-source-windows-for-500-record-hero` and is tracked in:

```text
docs/visual/reports/process-curve-external-scale-exporter.md
```

The most recent exporter quality follow-up is tracked in:

```text
docs/visual/reports/native-exporter-quality-audit-2026-06-26-subagent.md
docs/visual/agent-tasks/visual-exporter-cli-contract.md
docs/visual/agent-tasks/redif-native-public-evidence.md
docs/visual/agent-tasks/relation-matrix-reproducibility-audit.md
docs/visual/agent-tasks/redif-preview-integration.md
docs/visual/agent-tasks/process-curve-real-data-inventory.md
```

This follow-up is not visual polish. It is part of the evidence-library
contract. Current state: Process-Curve, Solver-Trace, Redif and Relation-Matrix
all produce parseable visual evidence by default; Redif is no longer labelled
synthetic; and the checked Relation-Matrix public asset is reproducible from the
native exporter. Redif now also has a native review-pending engine preview at
`visual/examples/redif-metric-dynamics-preview/index.html` that renders the
exported measure-path timeline through `showDynamics()` without JavaScript
algorithm computation.

The current external process-curve scale blocker was rechecked by local
inventory:

```text
docs/visual/reports/process-curve-real-data-inventory.md
```

The inventory found only the existing 48 real UCR-derived source windows. No
full local UCR archive or extracted source tree was found in the required local
search roots, so `missing-real-source-windows-for-500-record-hero` remains
active.

Scale-up implementation contracts for the native preview families are tracked
in:

```text
docs/visual/agent-tasks/condition-monitoring-scale-exporter.md
docs/visual/agent-tasks/mixed-records-scale-exporter.md
docs/visual/agent-tasks/cross-space-scale-exporter.md
docs/visual/agent-tasks/dynamics-scale-exporter.md
docs/visual/agent-tasks/mapping-dimensionality-scale-exporter.md
docs/visual/agent-tasks/process-curve-external-scale-exporter.md
docs/visual/agent-tasks/relation-matrix-hero-readability.md
```

## C++ Writer Core

Native exporter validation proves that a document is schema-valid and indexable.
It does not prove that the native export surface is production-quality.

The reusable writer core is tracked in:

```text
visual/cpp/mtrc_visual.hpp
docs/visual/agent-tasks/cpp-visual-export-core-workstream.md
```

Current status:

- `visual/cpp/mtrc_visual.hpp` provides the header-only C++17 writer foundation.
- `visual/tools/check-cpp-export.mjs` validates the standalone C++ helper path.
- All six foundation native visual exporters now route common
  JSON/document/file writing through `mtrc::visual`.
- Exporters may still use raw `visual::object(...)` composition for
  domain-specific evidence shapes that should not be flattened into generic
  helper methods.
- Default exporter CLI behavior is normalized across the newer Process-Curve,
  Solver-Trace and Redif exporters: no arguments mean JSON on stdout; file
  writes require explicit output arguments.

Future exporter changes should keep one exporter per worker and disjoint write
scopes.

## Acceptance For A Native Exporter

An exporter is integration-ready when:

- it compiles without adding runtime dependencies
- it emits `metric.visual.v1`
- `provenance.synthetic` is absent or false
- `node visual/tools/check-visual-document.mjs` accepts the exported JSON
- C++ computes all metric values, properties, diagnostics and coordinates
- JavaScript only validates, indexes and renders the exported result

An exporter is not a public hero yet. Public hero status requires a distinct
visual grammar, browser screenshot review, and the public gallery evidence gate.
