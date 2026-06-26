# Process-Curve Descriptor Factory Quarantine

## Call Sites Removed

- `visual/examples/miniature-hero-frame/index.html` no longer imports or calls `createProcessCurveMiniatureLayerDescriptors`. It now loads `docs/examples/assets/process-curve-external/metric.visual.json` and renders through `createMetricVisual(...).showProcessCurves(...)`.
- `visual/examples/miniature-look-gallery/index.html` no longer imports or calls `createProcessCurveMiniatureLayerDescriptors`. Its look atlas descriptors now come from `ProcessCurveSceneView.fromVisualSpace(...).toLayerDescriptors()`.
- Both pages stopped loading legacy `metric.evidence.v1` JSON and no longer call `adaptMetricEvidenceV1`.

## Internal Helper Status

- `createProcessCurveMiniatureLayerDescriptors` is private to `visual/src/views/ProcessCurveSceneView.js`.
- `visual/src/views/index.js` no longer exports the descriptor factory, so `visual/src/index.js` no longer exposes it through the public barrel.
- `ProcessCurveSceneView` remains the semantic owner for the process-curve field, ground projection, record-order trajectory, record-volume skyline glyphs, morph layer, optional neighborhood graph, and optional relation matrix.
- `visual/tools/check-process-curve-descriptor-factory-quarantine.mjs` now fails if any `visual/examples/*` page imports, references, or calls the quarantined factory.

## Validation

- PASS: `node --check visual/src/views/ProcessCurveSceneView.js`
- PASS: `node visual/tools/check-process-curve-descriptor-factory-quarantine.mjs`
- PASS: `node visual/tools/check-process-curve-scene-view.mjs`
- PASS: `node visual/tools/check-single-render-pipeline.mjs`
- PASS: `node visual/tools/check-views.mjs`
- PASS: `node visual/tools/check-visual-command-api.mjs`
- PASS: `node visual/tools/check-grae10-golden.mjs`
- PASS: `rg -n "createProcessCurveMiniatureLayerDescriptors" visual/examples visual/src/views/index.js visual/src/index.js` returned no matches.

## Direct-Runtime Pages Left

- `visual/examples/miniature-look-gallery/index.html` still uses the direct runtime/style atlas harness, but its descriptors now originate from `ProcessCurveSceneView`.
- `visual/examples/native-engine-probe/index.html` still uses the direct runtime harness.
- `visual/examples/grae10-metric-engine/` was left untouched as required.
