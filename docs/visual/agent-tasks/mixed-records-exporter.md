# Agent Task: Mixed Records Native Visual Exporter

## Owner

One worker owns only:

- `examples/engine/mixed_finite_records_visual_export.cpp`

The parent coordinator owns CMake, tests, docs integration, and commits.

## Inputs

Read:

- `examples/engine/mixed_finite_records.cpp`
- `examples/engine/mixed_finite_records.hpp`
- `examples/engine/mixed_finite_records_fixture.hpp`
- `visual/cpp/mtrc_visual.hpp`
- `docs/visual/metric-visual-v1-schema.md`

## Goal

Create a native C++ exporter that emits `metric.visual.v1` for the mixed finite
records workflow. The output is an evidence foundation, not a public hero yet.

## Required Behavior

- stdout emits valid `metric.visual.v1` by default.
- `--export-dir <dir>` writes `<dir>/metric.visual.json`.
- Provenance identifies native C++ export and must not set
  `provenance.synthetic: true`.
- Records come from the existing mixed finite records fixture.
- Include at least:
  - dataset metadata
  - mixed records with original payloads or compact payload summaries
  - one finite metric relation computed by `MixedRecordMetric`
  - one finite metric space
  - coordinates suitable for first visualization
  - categorical properties for family/type where applicable
  - scalar properties for severity and useful metric diagnostics
  - diagnostics mirroring important native checks where practical

## Constraints

- Do not add dependencies.
- Do not implement algorithms in Python or JavaScript.
- Do not edit CMake.
- Do not edit shared visual engine files.

## Acceptance

The coordinator must be able to run:

```bash
c++ -std=c++17 -I. -Ivisual/cpp examples/engine/mixed_finite_records_visual_export.cpp -o /tmp/mixed_visual_export
/tmp/mixed_visual_export --export-dir /tmp/mixed-visual-export
node visual/tools/check-visual-document.mjs /tmp/mixed-visual-export/metric.visual.json
```

