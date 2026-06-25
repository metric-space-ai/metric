# Agent Task: Process-Curve External Visual Exporter

## Purpose

Turn the existing UCR process-curve gallery evidence into a native
`metric.visual.v1` engine payload. This is the next real public hero candidate
after GRAE10 because the records and query outcomes already come from the
process-curve C++ example and existing CSV evidence, not from JavaScript
decoration.

The output must be useful to the reusable Visual Engine. Do not build a custom
SVG report, one-off page renderer, or fake screenshot.

## Owner Scope

One worker owns only:

```text
examples/engine/process_curve_external_gallery.cpp
examples/engine/process_curve_external_visual_export.cpp
examples/engine/CMakeLists.txt
docs/examples/assets/process-curve-external/metric.visual.json
visual/examples/process-curve-external-hero/index.html
visual/tools/check-process-curve-external-visual.mjs
docs/visual/reports/process-curve-external-visual-exporter.md
```

If a listed file does not exist, create it only when needed. Do not edit GRAE10,
project pages, unrelated exporters, visual runtime layers, or synthetic fixture
JSON.

## Required Behavior

- Use existing process-curve evidence only:
  - `examples/engine/assets/process_curve_power_demand_gallery.csv`
  - `examples/engine/assets/process_curve_internal_bleeding_gallery.csv`
  - or the already exported files under
    `docs/examples/assets/process-curve-external/`
- C++ must compute or load the evidence and write `metric.visual.v1`.
- JavaScript may validate, index and render the exported document only.
- The document must include:
  - records with original process-curve snippets as payloads
  - query records or query events with expected role/label
  - metric-space winner and padded-vector baseline outcome properties
  - at least one relation containing the aligned process-curve metric values
  - coordinates suitable for a visual grammar, derived from exported C++
    evidence rather than JS layout invention
  - a view definition that maps to a reusable command or semantic view
  - provenance showing native C++ generation and no synthetic public evidence
- The example page must call the Visual Engine command/API. It must not create
  page-local canvas, SVG, or HTML chart renderers.

## Stop Rules

- Stop and write the report if the required source data is missing.
- Stop if the implementation would invent records, labels, winners, distances,
  entropy, query results or coordinates in JavaScript.
- Stop if a public page would have to use a static SVG as the primary evidence.
- Do not touch `visual/examples/grae10-metric-engine/`.

## Acceptance

Run:

```bash
node visual/tools/check-visual-document.mjs docs/examples/assets/process-curve-external/metric.visual.json
node visual/tools/check-process-curve-external-visual.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-hero-grammar-contract.mjs
node visual/tools/check-single-render-pipeline.mjs
node visual/tools/check-grae10-golden.mjs
```

If CMake files are changed, also run the targeted CMake/CTest path for the new
exporter. If the existing build directory cannot be used, document the exact
blocker instead of marking the task done.

## Report

Write `docs/visual/reports/process-curve-external-visual-exporter.md` with:

- source files used
- generated record/query/relation counts
- C++ executable and output path
- Visual Engine command/view used by the example page
- commands run and results
- whether this is still preview-only or ready for screenshot review
