# Agent Task: Condition Monitoring Native Visual Exporter

## Owner

One worker owns only:

- `examples/engine/condition_monitoring_visual_export.cpp`

The parent coordinator owns CMake, tests, docs integration, generated assets,
gallery pages, and commits.

## Inputs

Read:

- `examples/engine/condition_monitoring.cpp`
- `docs/examples/condition-monitoring.md`
- `visual/cpp/mtrc_visual.hpp`
- `docs/visual/metric-visual-v1-schema.md`

## Goal

Create a native C++ exporter that emits `metric.visual.v1` for condition
monitoring evidence: process windows as records, metric relation, anomaly or
regime properties, coordinates, trajectories, and diagnostics.

## Required Behavior

- stdout emits valid `metric.visual.v1` by default.
- `--export-dir <dir>` writes `<dir>/metric.visual.json`.
- Provenance identifies native C++ export and must not set
  `provenance.synthetic: true`.
- Include at least:
  - records for process windows or state windows
  - a finite metric relation computed in C++
  - one finite metric space
  - coordinates suitable for miniature process-state visualization
  - scalar anomaly/regime/severity properties where available
  - categorical run/regime properties where available
  - diagnostics mirroring native condition-monitoring checks where practical

## Constraints

- Do not add dependencies.
- Do not compute condition-monitoring results in JavaScript or Python.
- Do not edit CMake.
- Do not edit shared visual engine files.
- Do not promote this to a public hero page. This task only creates native
  evidence.

## Acceptance

The coordinator must be able to run:

```bash
c++ -std=c++17 -I. -Ivisual/cpp examples/engine/condition_monitoring_visual_export.cpp -o /tmp/condition_monitoring_visual_export
/tmp/condition_monitoring_visual_export --export-dir /tmp/condition-monitoring-visual-export
node visual/tools/check-visual-document.mjs /tmp/condition-monitoring-visual-export/metric.visual.json
```
