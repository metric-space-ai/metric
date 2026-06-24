# Agent Task: Dynamics Native Visual Exporter

## Owner

One worker owns only:

- `examples/engine/finite_metric_dynamics_visual_export.cpp`

The parent coordinator owns CMake, tests, docs integration, and commits.

## Inputs

Read:

- `examples/engine/finite_metric_dynamics.cpp`
- `examples/engine/finite_metric_dynamics_fixture.hpp`
- `visual/cpp/mtrc_visual.hpp`
- `docs/visual/metric-visual-v1-schema.md`

## Goal

Create a native C++ exporter that emits `metric.visual.v1` for finite metric
dynamics: states, transitions, timelines and trajectories.

## Required Behavior

- stdout emits valid `metric.visual.v1` by default.
- `--export-dir <dir>` writes `<dir>/metric.visual.json`.
- Provenance identifies native C++ export and must not set
  `provenance.synthetic: true`.
- Include at least:
  - records/states from the native fixture
  - a finite metric relation or transition graph
  - coordinates or timeline coordinate states
  - scalar properties for step/state/uncertainty/transition cost where available
  - diagnostics mirroring native example checks where practical

## Constraints

- Do not add dependencies.
- Do not simulate dynamics in JavaScript.
- Do not edit CMake.
- Do not edit shared visual engine files.

## Acceptance

The coordinator must be able to run:

```bash
c++ -std=c++17 -I. -Ivisual/cpp examples/engine/finite_metric_dynamics_visual_export.cpp -o /tmp/dynamics_visual_export
/tmp/dynamics_visual_export --export-dir /tmp/dynamics-visual-export
node visual/tools/check-visual-document.mjs /tmp/dynamics-visual-export/metric.visual.json
```

