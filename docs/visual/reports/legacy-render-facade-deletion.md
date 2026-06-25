# Legacy Render Facade Deletion

## Deleted Legacy Symbols

Removed from `visual/src/metric-visual.js`:

- `loadMetricEvidence`
- `assertMetricEvidence`
- `MetricSelection`
- `MetricScene3D`
- `MetricRecordGallery`
- `MetricHeatmap`
- `MetricQueryInspector`
- `MetricProcessCurveApp`

Also deleted the helper code that only served those exports: raw WebGL program
and shader setup, old 2D canvas drawing helpers, old panel query helpers, and
orphaned camera/matrix math used by the deleted scene renderer.

`adaptMetricEvidenceV1()` remains in `visual/src/data/evidence-adapter.js`.
It is still compatibility data ingestion, not a public render facade.

## New Process-Curve Path

`visual/examples/process-curve-condition-monitoring/index.html` now loads:

```text
docs/examples/assets/process-curve-external/metric.visual.json
```

The page creates the public visual surface and renders via:

```text
createMetricVisual({ evidence: visualDocument, canvas })
  -> visual.showProcessCurves(...)
  -> semantic command/view
  -> layer descriptors
  -> MetricVisualRuntime
```

The page no longer imports or constructs `MetricVisualRuntime`, layer factories,
miniature scene bundles, legacy panel canvases, or old `metric.evidence.v1`
inputs. The morph controls rerun `visual.showProcessCurves(...)` with command
options rather than mutating renderer layers directly.

## Validation Results

- PASS: `rg -n "MetricScene3D|MetricRecordGallery|MetricHeatmap|MetricQueryInspector|MetricProcessCurveApp|MetricSelection|loadMetricEvidence|assertMetricEvidence" visual/src/metric-visual.js visual/examples/process-curve-condition-monitoring/index.html`
  - No matches. `rg` returned 1, which is the expected no-match result.
- PASS: `rg -n "metric\\.evidence\\.v1|evidence\\.json" visual/examples/process-curve-condition-monitoring/index.html`
  - No matches. `rg` returned 1, which is the expected no-match result.
- PASS: `node --check visual/src/metric-visual.js`
- PASS: `node --check visual/tools/check-legacy-render-facade-deletion.mjs`
- PASS: `node visual/tools/check-legacy-render-facade-deletion.mjs`
- PASS: `node visual/tools/check-single-render-pipeline.mjs`
- PASS: `node visual/tools/check-visual-command-api.mjs`
- PASS: `node visual/tools/check-runtime-picking-preview.mjs`
- PASS: `node visual/tools/check-relation-matrix-picker.mjs`
- PASS: `METRIC_VISUAL_EXAMPLES=process-curve-condition-monitoring node visual/tools/check-visual-regression-public-examples.mjs`
  - The checker completed successfully but discovered `total: 0` examples for
    this page because it is not currently part of the public regression set.
    `visual/tools/check-legacy-render-facade-deletion.mjs` was added as the
    narrow page checker for this task.
- PASS: `node visual/tools/check-grae10-golden.mjs`
  - Golden hash reported:
    `464f6a90c36c1e9c6b4ec90068500dc226740d65b251918aca567f99d64d3d5e`.

## Remaining Noncanonical Paths

No noncanonical render path remains in the owned public surface for this task.
Other internal probes, miniature galleries, and verification tools outside the
owner scope still have their own legacy fixture or preview coverage and were
left untouched for separate workstreams.
